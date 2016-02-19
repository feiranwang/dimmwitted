
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <zmq.hpp>
#include "app/cox/cox.h"
#include "timer.h"

dd::Cox::Cox(FactorGraph &fg, int n_epochs, std::string folder,
             double lr, double lr_decay, double lambda, double alpha,
             bool fusion_mode) :
  n_epochs(n_epochs),
  lr(lr),
  lr_decay(lr_decay),
  lambda(lambda),
  alpha(alpha),
  folder(folder),
  fusion_mode(fusion_mode) {
  // TODO fusion weight
  n_features = fg.n_weight;

  // count below
  n_train = 0;
  n_test = 0;

  // TODO variable id mapping for cnn

  // initialize beta
  for (int i = 0; i < n_features; i++) {
    beta.push_back(0);
  }
  for (int i = 0; i < n_features; i++) {
    // initialize from fixed weight or uniform [-1, 1]
    // double weight = fg.weights[i].isfixed ? fg.weights[i].weight : 2 * ((double)rand() / RAND_MAX) - 1;
    double weight = 0;
    beta[fg.weights[i].id] = weight;
  }

  // collect variables
  std::vector<long> vids;
  for (long i = 0; i < fg.n_var; i++) {
    if (fg.variables[i].is_observation) continue;
    vids.push_back(i);
  }

  // go through each variable in the fg and contruct Cox model
  for (size_t i = 0; i < vids.size(); i++) {

    Variable &variable = fg.variables[vids[i]];

    // find all features
    CompactFactor * const fs = &(fg.compact_factors[variable.n_start_i_factors]);
    std::vector<double> features(n_features, 0);
    for (int j = 0; j < variable.n_factors; j++) {
      // TODO assume y = 0 means y is satisfied
      long weight_id = fg.factors[fs[j].id].weight_id;
      features[weight_id] = fs[j].potential(fg.vifs, fg.infrs->assignments_evid, variable.id, 0);
    }

    if (variable.is_evid) {
      y_train.push_back(variable);
      x_train.push_back(features);
      idmap_train[variable.id] = n_train;
      n_train++;
    } else {
      y_test.push_back(variable);
      x_test.push_back(features);
      idmap_test[variable.id] = n_test;
      n_test++;
    }
  }

  n_vars = n_train + n_test;

  // std::cout << "n_train = " << n_train << " n_test = " << n_test << std::endl;
  // for (int i = 0; i < n_train; i++) {
  //   std::cout << "time = " << y_train[i].assignment_evid << " status = " << 1 - y_train[i].is_censored << std::endl;
  //   for (int j = 0; j < n_features; j++) {
  //     std::cout << x_train[i][j] << std::endl;
  //   }
  //   std::cout << std::endl;
  // }

  // solver settings
  momentum = 0.9;

  // fusion, handle only 1 cnn
  if (fusion_mode) {
    cnn_port = fg.cnn_ports[0];
    cnn_train_iterations = fg.cnn_train_iterations[0];
    cnn_test_iterations = fg.cnn_test_iterations[0];
    cnn_test_interval = fg.cnn_test_intervals[0];
    cnn_batch_size = fg.cnn_batch_sizes[0];
    // in Cox model, batch size must be equal to training set size
    assert(cnn_batch_size == n_train);
    for (int i = 0; i < n_train; i++) {
      cnn_scores_train.push_back(0);
    }
    for (int i = 0; i < n_test; i++) {
      cnn_scores_test.push_back(0);
    }
    this->n_epochs = (cnn_train_iterations / cnn_test_interval + 1) * cnn_test_iterations + cnn_train_iterations;
  }

}

std::vector<double> dd::Cox::compute_scores(std::vector<std::vector<double>> x, std::vector<double> cnn_scores) {
  int m = x.size();
  std::vector<double> scores(m, 0);
  for (int i = 0; i < m; i++) {
    double score = 0;
    for (int j = 0; j < n_features; j++) {
      score += x[i][j] * beta[j];
    }
    scores[i] = score;
  }
  if (fusion_mode) {
    for (int i = 0; i < m; i++) {
      scores[i] += cnn_scores[i];
    }
  }
  return scores;
}

std::vector<double> dd::Cox::compute_theta(std::vector<double> scores) {
  std::vector<double> theta(n_train, 0);
  for (int i = 0; i < n_train; i++) {
    theta[i] = exp(scores[i]);
  }
  return theta;
}

double dd::Cox::compute_loss() {
  std::vector<double> scores = compute_scores(x_train, cnn_scores_train);
  std::vector<double> theta = compute_theta(scores);
  double loss = 0;
  for (int i = 0; i < n_train; i++) {
    // only sum over events
    if (y_train[i].is_censored) continue;

    // second term of loss
    double sum_theta = 0;
    for (int j = 0; j < n_train; j++) {
      if (y_train[j].assignment_evid >= y_train[i].assignment_evid) {
        sum_theta += theta[j];
      }
    }
    loss += - scores[i] + log(sum_theta);
  }
  loss = loss / n_train;

  double norm_2 = 0, norm_1 = 0;
  for (int j = 0; j < n_features; j++) {
    norm_2 += beta[j] * beta[j];
    norm_1 += std::abs(beta[j]);
  }
  loss += lambda * (0.5 * (1 - alpha) * norm_2 + alpha * norm_1);
  return loss;
}

std::vector<double> dd::Cox::gradients_to_scores() {
  std::vector<double> gradients(n_train, 0);
  std::vector<double> scores = compute_scores(x_train, cnn_scores_train);
  std::vector<double> theta = compute_theta(scores);

  for (int i = 0; i < n_train; i++) {
    if (y_train[i].is_censored) continue;
    double sum_theta = 0;
    for (int j = 0; j < n_train; j++) {
      if (y_train[j].assignment_evid >= y_train[i].assignment_evid) {
        sum_theta += theta[j];
      }
    }

    for (int j = 0; j < n_train; j++) {
      gradients[j] += -(j == i) + (y_train[j].assignment_evid >= y_train[i].assignment_evid) * theta[j] / sum_theta;
    }

  }
  for (int i = 0; i < n_train; i++) {
    gradients[i] /= n_train;
  }
  return gradients;
}

std::vector<double> dd::Cox::gradients_to_beta() {
  std::vector<double> gradients(n_features, 0);
  std::vector<double> scores = compute_scores(x_train, cnn_scores_train);
  std::vector<double> theta = compute_theta(scores);

  for (int i = 0; i < n_train; i++) {
    if (y_train[i].is_censored) continue;
    double sum_theta = 0;
    for (int j = 0; j < n_train; j++) {
      if (y_train[j].assignment_evid >= y_train[i].assignment_evid) {
        sum_theta += theta[j];
      }
    }

    std::vector<double> sum_theta_x(n_features, 0);
    for (int j = 0; j < n_train; j++) {
      if (y_train[j].assignment_evid >= y_train[i].assignment_evid) {
        for (int k = 0; k < n_features; k++) {
          sum_theta_x[k] += theta[j] * x_train[j][k];
        }
      }
    }

    for (int j = 0; j < n_features; j++) {
      gradients[j] += -x_train[i][j] + sum_theta_x[j] / sum_theta;
    }
  }

  for (int j = 0; j < n_features; j++) {
    gradients[j] /= n_train;
  }

  // regularization terms
  for (int j = 0; j < n_features; j++) {
    gradients[j] += lambda * ((1 - alpha) * beta[j] + alpha * ((beta[j] > 0) - (beta[j] < 0)));
  }

  return gradients;
}

void dd::Cox::train() {

  // fusion
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REP);
  zmq::message_t request;

  if (fusion_mode) {
    socket.bind(("tcp://*:" + cnn_port).c_str());
  }


  Timer t_total;

  std::vector<double> cache(n_features, 0);

  for (int epoch = 0; epoch < n_epochs; epoch++) {
    Timer t;
    t.restart();

    // receive fusion message, and save cnn scores
    if (fusion_mode) {

      socket.recv(&request);
      FusionMessage *msg = (FusionMessage *)request.data();

      // for (int j = 0; j < msg->batch; j++) {
      //   printf("vid = %d label = %d\n", msg->imgids[j], msg->labels[j]);
      // }

      if (msg->msg_type == REQUEST_GRAD) {
        save_fusion_message(msg, true);
      } else if (msg->msg_type == REQUEST_ACCURACY) {
        save_fusion_message(msg, false);
        socket.send(request);
        continue;
      }
    }

    double loss = compute_loss();
    std::vector<double> gradients = gradients_to_beta();

    // reply fusion message, backprop cnn gradients during training, and original data during testing
    if (fusion_mode) {
      FusionMessage *msg = (FusionMessage *)request.data();
      if (msg->msg_type == REQUEST_GRAD) {
        std::vector<double> gradients_scores = gradients_to_scores();
        for (int i = 0; i < n_train; i++) {
          msg->content[i] = gradients_scores[i];
        }
        socket.send(request);
      }
    }

    // momentum-GD
    for (int i = 0; i < n_features; i++) {
      cache[i] = momentum * cache[i] - lr * gradients[i];
    }
    for (int i = 0; i < n_features; i++) {
      beta[i] += cache[i];
    }
    std::cout << "Epoch " << epoch << ", loss = " << loss << std::endl;
    // for (int i = 0; i < n_features; i++) {
    //   std::cout << gradients[i] << ", ";
    // }
    // std::cout << std::endl;
    std::cout << "    time = " << t.elapsed() << " sec." << std::endl;

    // lr decay
    lr *= lr_decay;
  }
  // for (int i = 0; i < n_features; i++) {
  //   std::cout << beta[i] << ", ";
  // }
  // std::cout << std::endl;

  std::cout << "Total training time = " << t_total.elapsed() << std::endl;

  dump_weights();
}

void dd::Cox::dump_weights() {
  std::string filename = folder + "/inference_result.out.weights.text";
  std::ofstream fout(filename.c_str());
  for (int i = 0; i < n_features; i++) {
    fout << i << " " << beta[i] << std::endl;
  }
  fout.close();
}

void dd::Cox::test() {
  std::string filename = folder + "/inference_result.out.text";
  std::ofstream fout(filename.c_str());

  dump_scores_helper(fout, x_train, y_train, cnn_scores_train);
  dump_scores_helper(fout, x_test, y_test, cnn_scores_test);
  fout.close();
}

void dd::Cox::dump_scores_helper(std::ofstream &fout, std::vector<std::vector<double>> x, std::vector<Variable> y,
  std::vector<double> cnn_scores) {
  std::vector<double> scores = compute_scores(x, cnn_scores);
  for (size_t i = 0; i < scores.size(); i++) {
    fout << y[i].id << " 0 " << scores[i] << std::endl;
  }
}

void dd::Cox::save_fusion_message(FusionMessage *msg, bool train) {
  if (train) {
    assert(msg->nelem == n_train);
    assert(msg->batch == n_train);
    for (int i = 0; i < n_train; i++) {
      long vid = msg->imgids[i];
      cnn_scores_train[idmap_train[vid]] = msg->content[i];
    }
  } else {
    assert(msg->nelem == n_test);
    assert(msg->batch == n_test);
    for (int i = 0; i < n_test; i++) {
      long vid = msg->imgids[i];
      cnn_scores_test[idmap_train[vid]] = msg->content[i];
    }
  }
}

