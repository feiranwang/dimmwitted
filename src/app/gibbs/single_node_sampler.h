
#include "worker/single_node_worker.h"
#include "app/gibbs/single_thread_sampler.h"
#include <stdlib.h>
#include <numa.h>

#ifndef _SINGLE_NODE_SAMPLER_H
#define _SINGLE_NODE_SAMPLER_H

namespace dd{
  void gibbs_single_thread_task(CompactFactorGraph * const _p_fg, int i_worker, int n_worker){
    //numa_set_localalloc();
    SingleThreadSampler sampler = SingleThreadSampler(_p_fg);
    sampler.sample(i_worker,n_worker);
  }

  void gibbs_single_thread_tally_factortask(CompactFactorGraph * const _p_fg, int i_worker, int n_worker){
    //numa_set_localalloc();
    SingleThreadSampler sampler = SingleThreadSampler(_p_fg);
    sampler.tally_factors(i_worker,n_worker);
  }

  class SingleNodeSampler{

  public:
    CompactFactorGraph * const p_fg;

    int nthread;
    int nodeid;

    SingeNodeWorker<CompactFactorGraph, gibbs_single_thread_task> * sample_worker;
    SingeNodeWorker<CompactFactorGraph, gibbs_single_thread_tally_factortask> * tally_worker;

    SingleNodeSampler(CompactFactorGraph * _p_fg, int _nthread, int _nodeid) :
      p_fg (_p_fg)    
    {
      this->nthread = _nthread;
      this->nodeid = _nodeid;
      this->sample_worker = new SingeNodeWorker<CompactFactorGraph, gibbs_single_thread_task>(this->p_fg, 
        this->nthread, this->nodeid);
      this->tally_worker = new SingeNodeWorker<CompactFactorGraph, gibbs_single_thread_tally_factortask>(this->p_fg, 
        this->nthread, this->nodeid);
    }

    void clear_variabletally(){
      for(size_t i=0;i<this->p_fg->n_variables;i++){
        p_fg->fg_mutable->agg_means[i] = 0.0;
        p_fg->fg_mutable->agg_nsamples[i] = 0.0;
      }
    }

    void clear_factortally(){
      for(size_t i=0;i<this->p_fg->n_factors;i++){
        p_fg->fg_mutable->factor_tallies_free[i] = 0.0;
        p_fg->fg_mutable->factor_tallies_evid[i] = 0.0;
      }
    }

    void set_does_change_evid(const bool & does_change_evid){
      this->p_fg->does_change_evid = does_change_evid;
    }

    void sample(){
      this->sample_worker->execute();
    }

    void wait(){
      this->sample_worker->wait();
    }

    void tally_factor(){
      this->tally_worker->execute();
    }

    void wait_tally_factor(){
      this->tally_worker->wait();
    }

  };
}

#endif






