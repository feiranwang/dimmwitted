
#include "worker/single_node_worker.h"
#include "app/gibbs/single_thread_sampler.h"
#include <stdlib.h>

#ifndef _SINGLE_NODE_SAMPLER_H
#define _SINGLE_NODE_SAMPLER_H

namespace dd{
  void gibbs_single_thread_task(CompactFactorGraph * const _p_fg, int i_worker, int n_worker){
    SingleThreadSampler sampler = SingleThreadSampler(_p_fg);
    sampler.sample(i_worker,n_worker);
  }

  class SingleNodeSampler{

  public:
    CompactFactorGraph * const p_fg;

    double * const factor_tallies;

    double * const factor_tallies_free_evid;

    int nthread;
    int nodeid;

    SingeNodeWorker<CompactFactorGraph, gibbs_single_thread_task> * worker;

    SingleNodeSampler(CompactFactorGraph * _p_fg, int _nthread, int _nodeid) :
      p_fg (_p_fg), 
      factor_tallies(new double[_p_fg->n_factors]),
      factor_tallies_free_evid(new double[_p_fg->n_factors])
    {
      this->nthread = _nthread;
      this->nodeid = _nodeid;
      this->worker = new SingeNodeWorker<CompactFactorGraph, gibbs_single_thread_task>(this->p_fg, 
        this->nthread, this->nodeid);
    }

    void clear_factortally(){
      for(int i=0;i<this->p_fg->n_factors;i++){
        factor_tallies[i] = 0.0;
        factor_tallies_free_evid[i] = 0.0;
      }
    }

    void ensure_evids(){
      this->p_fg->ensure_evids();
    }

    void sample(){
      this->p_fg->does_change_evid = false;
      this->worker->execute();
    }

    void wait(){
      this->worker->wait();
      for(int i=0;i<this->p_fg->n_factors;i++){
        factor_tallies[i] += this->p_fg->potential(this->p_fg->factors[i]);
      }
    }

    void sample_free_evid(){
      this->p_fg->does_change_evid = true;
      this->worker->execute();
    }

    void wait_free_evid(){
      this->worker->wait();
      for(int i=0;i<this->p_fg->n_factors;i++){
        factor_tallies_free_evid[i] += this->p_fg->potential(this->p_fg->factors[i]);
      }
    }

  };
}

#endif






