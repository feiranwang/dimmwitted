
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
  n_thread_per_numa = 6;

  for(int i=0;i<=n_numa_nodes;i++){
    this->compact_factors.push_back(dd::CompactFactorGraph(this->p_fg, i));
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

  for(int i_epoch=0;i_epoch<n_epoch;i_epoch++){

    t.restart();
    std::cout << "EPOCH " << i_epoch << "...." << std::endl;
    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].sample();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait();
    }
    double elapsed = t.elapsed();
    std::cout << "     "  << elapsed << " seconds" << std::endl;
    std::cout << "     "  << (nvar*nnode)/elapsed << " samples/seconds" << std::endl;
  }
}

void dd::GibbsSampling::learn(const int & n_epoch, const int & n_sample_per_epoch, 
                              const double & stepsize){

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
      single_node_samplers[i].ensure_evids();
      single_node_samplers[i].clear_factortally();
    }

    std::cout << "EPOCH " << i_epoch << "...." << std::endl;
    
    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].sample();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].sample_free_evid();
    }

    for(int i=0;i<=n_numa_nodes;i++){
      single_node_samplers[i].wait_free_evid();
    }

    for(int i=1;i<=n_numa_nodes;i++){
      CompactFactorGraph const & cfg = this->compact_factors[i];
      for(int j=0;j<cfg.n_factors;j++){
        single_node_samplers[0].factor_tallies[j] +=
          single_node_samplers[i].factor_tallies[j];
        single_node_samplers[0].factor_tallies_free_evid[j] +=
          single_node_samplers[i].factor_tallies_free_evid[j];
      }
    }


    double diff_exp;
    CompactFactorGraph const & cfg = this->compact_factors[0];
    for(int i=0;i<cfg.n_factors;i++){
      diff_exp = single_node_samplers[0].factor_tallies_free_evid[i]
        - single_node_samplers[0].factor_tallies[i];
      cfg.weights[cfg.factors[i].weight_id] -= stepsize * diff_exp;
    }
    std::cout << cfg.weights[0] << std::endl;

    for(int i=1;i<=n_numa_nodes;i++){
      CompactFactorGraph const & cfg = this->compact_factors[i];
      for(int j=0;j<cfg.n_weights;j++){
        cfg.weights[j] = this->compact_factors[0].weights[j];
      }
    }
    
    double elapsed = t.elapsed();
    std::cout << "     " << elapsed << " seconds" << std::endl;
    std::cout << "     "  << 2*(nvar*nnode)/elapsed << " samples/seconds" << std::endl;

  }

}

void dd::GibbsSampling::dump_weights(){
  CompactFactorGraph const & cfg = this->compact_factors[0];
  for(int i=0;i<cfg.n_weights;i++){
    std::cout << i << " " << cfg.weights[i] << std::endl;
  }
}


void dd::GibbsSampling::dump(){

  for(int i=0;i<=n_numa_nodes;i++){
    const CompactFactorGraph & cfg = compact_factors[i];
    for(auto & variable : this->p_fg->variables){
      variable.agg_mean += cfg.variables[variable.id].agg_mean;
      variable.n_sample += cfg.variables[variable.id].n_sample;
    }
  }


  int ct = 0;
  for(const auto & variable : this->p_fg->variables){
    ct ++;
    std::cout << variable.id << " " 
              << variable.agg_mean/variable.n_sample << "  @  " << variable.n_sample << std::endl;
    if(ct % 10 == 0){
      break;
    }
  }
}










