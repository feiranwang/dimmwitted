
#include "app/gibbs/gibbs_sampling.h"
#include "app/gibbs/single_node_sampler.h"
#include <numa.h>
#include <unistd.h>
#include "timer.h"

/*!
 * \brief In this function, the factor graph is located to each NUMA node.
 * 
 * TODO: in the near future, this allocation should be abstracted
 * into a higher-level class to avoid writing similar things
 * for Gibbs, NN, SGD etc. However, this is the task of next pass
 * of refactoring.
 */
void dd::GibbsSampling::prepare(){

  n_numa_nodes = numa_max_node();
  n_thread_per_numa = 20;

  for(int i=0;i<=n_numa_nodes;i++){

    numa_run_on_node(i);
    numa_set_localalloc();

    dd::CompactFactorGraph_Immutable * p_fg_immutable = 
      new dd::CompactFactorGraph_Immutable(this->p_fg, i);

    this->compact_factors.push_back(dd::CompactFactorGraph(
      p_fg_immutable,
      new dd::CompactFactorGraph_Mutable(this->p_fg, i)
    ));
  }

}

void dd::GibbsSampling::inference(const int & n_epoch){

  Timer t_total;

  Timer t;
  int nvar = this->compact_factors[0].n_variables;
  int nnode = n_numa_nodes + 1;

  std::vector<SingleNodeSampler> single_node_samplers;
  for(int i=0;i<=n_numa_nodes;i++){
    single_node_samplers.push_back(SingleNodeSampler(&this->compact_factors[i], 
      n_thread_per_numa, i));
  }

  for(int i=0;i<=n_numa_nodes;i++){
    single_node_samplers[i].clear_variabletally();
  }

  for(int i_epoch=0;i_epoch<n_epoch;i_epoch++){

    
    std::cout << std::setprecision(2) << "INFERENCE EPOCH " << i_epoch * nnode <<  "~" 
      << ((i_epoch+1) * nnode) << "...." << std::flush;

    t.restart();

    for(int i=0;i<nnode;i++){
      single_node_samplers[i].sample();
    }

    for(int i=0;i<nnode;i++){
      single_node_samplers[i].wait();
    }
    double elapsed = t.elapsed();
    std::cout << ""  << elapsed << " sec." ;
    std::cout << ","  << (nvar*nnode)/elapsed << " vars/sec" << std::endl;
  }

  double elapsed = t_total.elapsed();
  std::cout << "TOTAL INFERENCE TIME: " << elapsed << " sec." << std::endl;

}

void dd::GibbsSampling::learn(const int & n_epoch, const int & n_sample_per_epoch, 
                              const double & stepsize, const double & decay){

  Timer t_total;

  double current_stepsize = stepsize;

  Timer t;
  int nvar = this->compact_factors[0].n_variables;
  int nnode = n_numa_nodes + 1;
  int nweight = this->compact_factors[0].n_weights;

  //n_thread_per_numa = 1;

  std::vector<SingleNodeSampler> single_node_samplers;
  for(int i=0;i<=n_numa_nodes;i++){
    single_node_samplers.push_back(SingleNodeSampler(&this->compact_factors[i], n_thread_per_numa, i));
  }

  double * ori_weights = new double[nweight];

  for(int i_epoch=0;i_epoch<n_epoch;i_epoch++){

    std::cout << std::setprecision(2) << "LEARNING EPOCH " << i_epoch * nnode <<  "~" 
      << ((i_epoch+1) * nnode) << "...." << std::flush;

    t.restart();
    
    memcpy(ori_weights, this->compact_factors[0].weights, sizeof(double)*nweight);

    for(int i=0;i<nnode;i++){
      single_node_samplers[i].p_fg->stepsize = current_stepsize;
    }

    for(int i=0;i<nnode;i++){
      single_node_samplers[i].sample_sgd();
    }

    for(int i=0;i<nnode;i++){
      single_node_samplers[i].wait_sgd();
    }

    CompactFactorGraph const & cfg = this->compact_factors[0];
    for(int i=1;i<=n_numa_nodes;i++){
      CompactFactorGraph const & cfg_other = this->compact_factors[i];
      for(int j=0;j<cfg.n_weights;j++){
        cfg.fg_mutable->weights[j] += cfg_other.fg_mutable->weights[j];
      }
    }

    for(int j=0;j<cfg.n_weights;j++){
      cfg.fg_mutable->weights[j] /= nnode;
      cfg.fg_mutable->weights[j] *= (1.0/(1.0+0.01*current_stepsize));
    }

    for(int i=1;i<=n_numa_nodes;i++){
      CompactFactorGraph const & cfg_other = this->compact_factors[i];
      for(int j=0;j<cfg_other.n_weights;j++){
        if(cfg_other.fg_immutable->weights_is_fixed[j] == false){
          cfg_other.fg_mutable->weights[j] = this->compact_factors[0].fg_mutable->weights[j];
        }
      }
    }

    double lmax = -1000000;
    double l2=0.0;
    for(int i=0;i<nweight;i++){
      double diff = fabs(ori_weights[i] - cfg.fg_mutable->weights[i]);
      l2 += diff*diff;
      if(lmax < diff){
        lmax = diff;
      }
    }
    lmax = lmax/current_stepsize;
    
    double elapsed = t.elapsed();
    std::cout << "" << elapsed << " sec.";
    std::cout << ","  << (nvar*nnode)/elapsed << " vars/sec." << ",stepsize=" << current_stepsize << ",lmax=" << lmax << ",l2=" << sqrt(l2)/current_stepsize << std::endl;
    //std::cout << "     " << this->compact_factors[0].fg_mutable->weights[0] << std::endl;

    current_stepsize = current_stepsize * decay;

  }

  double elapsed = t_total.elapsed();
  std::cout << "TOTAL LEARNING TIME: " << elapsed << " sec." << std::endl;

}

void dd::GibbsSampling::dump_weights(){

  std::cout << "LEARNING SNIPPETS (QUERY WEIGHTS):" << std::endl;
  CompactFactorGraph const & cfg = this->compact_factors[0];
  int ct = 0;
  for(int i=0;i<cfg.n_weights;i++){
    ct ++;
    std::cout << "   " << i << " " << cfg.fg_mutable->weights[i] << std::endl;
    if(ct % 10 == 0){
      break;
    }
  }
  std::cout << "   ..." << std::endl; 
}


void dd::GibbsSampling::dump(){

  for(int i=0;i<=n_numa_nodes;i++){
    const CompactFactorGraph & cfg = compact_factors[i];
    for(auto & variable : this->p_fg->variables){
      variable.agg_mean += cfg.fg_mutable->agg_means[variable.id];
      variable.n_sample += cfg.fg_mutable->agg_nsamples[variable.id];
    }
  }

  std::cout << "INFERENCE SNIPPETS (QUERY VARIABLES):" << std::endl;
  int ct = 0;
  for(const auto & variable : this->p_fg->variables){
    if(variable.is_evid == false){
      ct ++;
      std::cout << "   " << variable.id << " " 
                << variable.agg_mean/variable.n_sample << "  @  " << variable.n_sample << std::endl;
      if(ct % 10 == 0){
        break;
      }
    }
  }
  std::cout << "   ..." << std::endl; 

  std::cout << "INFERENCE CALIBRATION (QUERY BINS):" << std::endl;
  std::vector<int> abc;
  for(int i=0;i<=10;i++){
    abc.push_back(0);
  }
  int bad = 0;
  for(const auto & variable : this->p_fg->variables){
    if(variable.is_evid == true){
      continue;
    }
    int bin = (int)(variable.agg_mean/variable.n_sample*10);
    if(bin >= 0 && bin <=10){
      abc[bin] ++;
    }else{
      bad ++;
    }
  }
  abc[9] += abc[10];
  for(int i=0;i<10;i++){
    std::cout << "PROB BIN 0." << i << "~0." << (i+1) << "  -->  # " << abc[i] << std::endl;
  }
  std::cout << std::endl;
  std::cout << "# BAD PROB " << bad << std::endl;


}










