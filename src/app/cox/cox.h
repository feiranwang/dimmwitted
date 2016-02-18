
#include <map>
#include "io/cmd_parser.h"
#include "common.h"
#include "dstruct/factor_graph/factor_graph.h"

#ifndef _COX_H_
#define _COX_H_

namespace dd {
  /**
   * Class for cox ph model
   */
  class Cox {
  public:

    // countings for components of
    long n_vars;
    long n_features;

    // number of evidence variables, query variables
    long n_train;
    long n_test;

    // weights, features, variables
    std::vector<double> beta;
    std::vector<std::vector<double>> x_train;
    std::vector<std::vector<double>> x_test;
    std::vector<Variable> y_train;
    std::vector<Variable> y_test;
    // map from variable id -> training matrix index
    std::map<long, int> idmap;

    // solver related settings
    int n_epochs;
    double lr;
    double lr_decay;
    double momentum;
    double lambda;
    double alpha;

    // output
    std::string folder;

    // fusion
    bool fusion_mode;
    std::string cnn_port;
    int cnn_train_iterations;
    int cnn_test_iterations;
    int cnn_test_interval;
    int cnn_batch_size;
    std::vector<double> cnn_scores_train; // scores from cnn
    std::vector<double> cnn_scores_test; // scores from cnn

    // Constructs a new cox model from factor graph
    Cox(FactorGraph &fg, int n_epochs, std::string folder, double lr, double lr_decay,
        double lambda, double alpha,
        bool fusion_mode);

    // scores
    std::vector<double> compute_scores(std::vector<std::vector<double>> x, std::vector<double> cnn_scores);

    // theta vector
    std::vector<double> compute_theta(std::vector<double> scores);

    // loss
    double compute_loss();

    // gradients
    std::vector<double> gradients_to_scores();
    std::vector<double> gradients_to_beta();

    // train
    void train();

    void dump_weights();

    void test();

    void dump_scores_helper(std::ofstream &fout, std::vector<std::vector<double>> x, std::vector<Variable> y,
      std::vector<double> cnn_scores);

  };
}

#endif
