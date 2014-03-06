
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

    double potential_pos_freeevid;
    double potential_neg_freeevid;

    double proposal_freevid;
    double proposal_fixevid;

    double r;

    drand48_data * const p_rand_seed;
    double * const p_rand_obj_buf;

    SingleThreadSampler(CompactFactorGraph * _p_fg) :
      p_fg (_p_fg), p_rand_seed(new drand48_data), p_rand_obj_buf(new double){
      srand48_r(rand(),this->p_rand_seed);
    }

    void sample(const int & i_sharding, const int & n_sharding){
      size_t nvar = p_fg->n_variables;
      size_t start = (nvar/n_sharding) * i_sharding;
      size_t end = (nvar/n_sharding) * (i_sharding+1);
      end = end > nvar ? nvar : end;
      for(size_t i=start; i<end; i++){
        this->sample_single_variable(i);
      }
    }

    void sample_sgd(const int & i_sharding, const int & n_sharding){
      size_t nvar = p_fg->n_variables;
      size_t start = (nvar/n_sharding) * i_sharding;
      size_t end = (nvar/n_sharding) * (i_sharding+1);
      end = end > nvar ? end : nvar;
      for(size_t i=start; i<end; i++){
        this->sample_sgd_single_variable(i);
      }
    }

  private:

    void sample_sgd_single_variable(long vid){
      const CompactVariable & variable = this->p_fg->fg_immutable->variables[vid];

      if(variable.domain_type == DTYPE_BOOLEAN){

          //_mm_prefetch((void*) &p_fg->variable_assignments_free[variable.id], _MM_HINT_T1);
          //_mm_prefetch((void*) &p_fg->variable_assignments_evid[variable.id], _MM_HINT_T1);
          //_mm_prefetch((void*) &p_fg->agg_means[variable.id] , _MM_HINT_T1);
          //_mm_prefetch((void*) &p_fg->agg_nsamples[variable.id] , _MM_HINT_T1);

          potential_pos = p_fg->potential(variable, 1, false);
          potential_neg = p_fg->potential(variable, 0, false);

          //std::cout << potential_pos << " " << potential_neg << std::endl;

          if(variable.is_evid == false){
            drand48_r(this->p_rand_seed, this->p_rand_obj_buf);
            if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg-potential_pos)) < 1.0){
              this->p_fg->update(variable, 1.0, false);
            }else{
              this->p_fg->update(variable, 0.0, false);
            }
          }

          potential_pos_freeevid = p_fg->potential(variable, 1, true);
          potential_neg_freeevid = p_fg->potential(variable, 0, true);

          drand48_r(this->p_rand_seed, this->p_rand_obj_buf);
          if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg_freeevid-potential_pos_freeevid)) < 1.0){
            this->p_fg->update(variable, 1.0, true);
          }else{
             this->p_fg->update(variable, 0.0, true);
          }

          this->p_fg->update_weight(variable);
          
          //std::cout << proposal_fixevid << "   " << proposal_freevid << std::endl;

      }else{
        std::cout << "[ERROR] Only Boolean variables are supported now!" << std::endl;
        assert(false);
      }

    }

    void sample_single_variable(long vid){

      const CompactVariable & variable = this->p_fg->fg_immutable->variables[vid];

      if(variable.domain_type == DTYPE_BOOLEAN){

        if(variable.is_evid == false){

          potential_pos = p_fg->potential(variable, 1, false);
          potential_neg = p_fg->potential(variable, 0, false);

          drand48_r(this->p_rand_seed, this->p_rand_obj_buf);
          if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg-potential_pos)) < 1.0){
            this->p_fg->update(variable, 1.0, false);
          }else{
            this->p_fg->update(variable, 0.0, false);
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

