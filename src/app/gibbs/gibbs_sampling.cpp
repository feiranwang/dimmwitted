
#include "app/gibbs/gibbs_sampling.h"
#include "app/gibbs/single_node_sampler.h"
#include <numa.h>
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

  dd::CompactFactorGraph_Immutable * p_fg_immutable = 
      new dd::CompactFactorGraph_Immutable(this->p_fg, 0);

  for(int i=0;i<=n_numa_nodes;i++){
    this->compact_factors.push_back(dd::CompactFactorGraph(
      p_fg_immutable,
      new dd::CompactFactorGraph_Mutable(this->p_fg, i)
    ));
  }

}

void dd::GibbsSampling::inference(const int & n_epoch){

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
      single_node_samplers[i].set_does_change_evid(false);
      single_node_samplers[i].sample();
    }

    for(int i=0;i<nnode;i++){
      single_node_samplers[i].wait();
    }
    double elapsed = t.elapsed();
    std::cout << ""  << elapsed << " sec." ;
    std::cout << ","  << (nvar*nnode)/elapsed << " vars/sec" << std::endl;
  }
}

void dd::GibbsSampling::learn(const int & n_epoch, const int & n_sample_per_epoch, 
                              const double & stepsize, const double & decay){

  double current_stepsize = stepsize;

  Timer t;
  int nvar = this->compact_factors[0].n_variables;
  int nnode = n_numa_nodes + 1;

  std::vector<SingleNodeSampler> single_node_samplers;
  for(int i=0;i<=n_numa_nodes;i++){
    single_node_samplers.push_back(SingleNodeSampler(&this->compact_factors[i], n_thread_per_numa, i));
  }

  for(int i_epoch=0;i_epoch<n_epoch;i_epoch++){

    t.restart();

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].clear_factortally();
    }

    std::cout << std::setprecision(2) << "LEARNING EPOCH " << i_epoch << "...." << std::flush;
    
    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].set_does_change_evid(false);
      single_node_samplers[i].sample();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].tally_factor();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait_tally_factor();
    }

    
    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].set_does_change_evid(true);
      single_node_samplers[i].sample();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].tally_factor();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait_tally_factor();
    }

       
    for(int i=1;i<=n_numa_nodes;i++){
      CompactFactorGraph const & cfg = this->compact_factors[i];
      for(int j=0;j<cfg.n_factors;j++){
        single_node_samplers[0].p_fg->fg_mutable->factor_tallies_free[j] +=
          single_node_samplers[i].p_fg->fg_mutable->factor_tallies_free[j];
        single_node_samplers[0].p_fg->fg_mutable->factor_tallies_evid[j] +=
          single_node_samplers[i].p_fg->fg_mutable->factor_tallies_evid[j];
      }
    }

    double sum1 = 0.0;
    double sum2 = 0.0;
    double diff_exp;
    CompactFactorGraph const & cfg = this->compact_factors[0];
    for(int i=0;i<cfg.n_factors;i++){
      diff_exp = single_node_samplers[0].p_fg->fg_mutable->factor_tallies_evid[i]
        - single_node_samplers[0].p_fg->fg_mutable->factor_tallies_free[i];
      sum1 += single_node_samplers[0].p_fg->fg_mutable->factor_tallies_evid[i];
      sum2 += single_node_samplers[0].p_fg->fg_mutable->factor_tallies_free[i];
      cfg.fg_mutable->weights[cfg.fg_immutable->factors[i].weight_id] += current_stepsize * diff_exp;
    }

    //std::cout << "EVID = " << sum1 << "     FREE = " << sum2 << std::endl;
    
    for(int i=1;i<=n_numa_nodes;i++){
      CompactFactorGraph const & cfg = this->compact_factors[i];
      for(int j=0;j<cfg.n_weights;j++){
        if(cfg.fg_immutable->weights_is_fixed[j] == false){
          cfg.fg_mutable->weights[j] = this->compact_factors[0].fg_mutable->weights[j];
        }
      }
    }
    

    double elapsed = t.elapsed();
    std::cout << "" << elapsed << " sec.";
    std::cout << ","  << 2*(nvar*nnode)/elapsed << " vars/sec." << std::endl;
    //std::cout << "     " << this->compact_factors[0].fg_mutable->weights[0] << std::endl;

    current_stepsize = current_stepsize * decay;

  }

}

void dd::GibbsSampling::dump_weights(){

  std::cout << "LEARNING SNIPPETS (QUERY WEIGHTS):" << std::endl;
  CompactFactorGraph const & cfg = this->compact_factors[0];
  int ct = 0;
  for(int i=0;i<cfg.n_weights;i++){
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
}










