
#include "dstruct/factor_graph.h"
#include "timer.h"

#ifndef _SINGLE_THREAD_SAMPLER_H
#define _SINGLE_THREAD_SAMPLER_H

namespace dd{

  class SingleThreadSampler{

  public:
    CompactFactorGraph * const p_fg;

    double potential_pos;
    double potential_neg;
    double r;

    drand48_data * const p_rand_seed;
    double * const p_rand_obj_buf;

    SingleThreadSampler(CompactFactorGraph * _p_fg) :
      p_fg (_p_fg), p_rand_seed(new drand48_data), p_rand_obj_buf(new double){
      srand48_r(rand(),this->p_rand_seed);
    }

    void sample(const int & i_sharding, const int & n_sharding){
      //Timer t;
      size_t nvar = p_fg->n_variables;
      size_t start = (nvar/n_sharding) * i_sharding;
      size_t end = (nvar/n_sharding) * (i_sharding+1);
      end = end > nvar ? nvar : end;
      for(size_t i=start; i<end; i++){
        this->sample_single_variable(i);
      }
      //std::cout << t.elapsed() << std::endl;
    }

    void tally_factors(const int & i_sharding, const int & n_sharding){
      size_t nfactor = p_fg->n_factors;
      size_t start = (nfactor/n_sharding) * i_sharding;
      size_t end = (nfactor/n_sharding) * (i_sharding+1);
      end = end > nfactor ? nfactor : end;
      for(size_t i=start; i<end; i++){
        this->tally_single_factor(i);
      }
    }

  private:

    void tally_single_factor(long fid){
      CompactFactor & factor = p_fg->fg_immutable->factors[fid];

      if(this->p_fg->does_change_evid == true){
        p_fg->fg_mutable->factor_tallies_free[fid] += p_fg->potential(factor);
      }else{
        p_fg->fg_mutable->factor_tallies_evid[fid] += p_fg->potential(factor);
      }

    }

    void sample_single_variable(long vid){

      const CompactVariable & variable = this->p_fg->fg_immutable->variables[vid];

      if(variable.domain_type == DTYPE_BOOLEAN){

        if(variable.is_evid == false || this->p_fg->does_change_evid == true){

          //_mm_prefetch((void*) &p_fg->variable_assignments_free[variable.id], _MM_HINT_T1);
          //_mm_prefetch((void*) &p_fg->variable_assignments_evid[variable.id], _MM_HINT_T1);
          //_mm_prefetch((void*) &p_fg->agg_means[variable.id] , _MM_HINT_T1);
          //_mm_prefetch((void*) &p_fg->agg_nsamples[variable.id] , _MM_HINT_T1);

          potential_pos = p_fg->potential(variable, 1);
          potential_neg = p_fg->potential(variable, 0);

          //std::cout << potential_pos << " " << potential_neg << std::endl;

          drand48_r(this->p_rand_seed, this->p_rand_obj_buf);
          if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg-potential_pos)) < 1.0){
            this->p_fg->update(variable, 1.0);
          }else{
            this->p_fg->update(variable, 0.0);
          }

        }

      }else{
        std::cout << "[ERROR] Only Boolean variables are supported now!" << std::endl;
        assert(false);
      }


    }


  };

}


#endif

