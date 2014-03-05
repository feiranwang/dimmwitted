
#include "dstruct/factor_graph.h"

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
      size_t nvar = p_fg->n_variables;
      size_t start = (nvar/n_sharding) * i_sharding;
      size_t end = (nvar/n_sharding) * (i_sharding+1);
      end = end > nvar ? nvar : end;
      for(size_t i=start; i<end; i++){
        this->sample_single_variable(i);
      }
    }

  private:

    void sample_single_variable(long vid){

      CompactVariable & variable = this->p_fg->variables[vid];

      if(variable.domain_type == DTYPE_BOOLEAN){

        if(variable.is_evid == false || this->p_fg->does_change_evid == true){

          potential_pos = p_fg->potential(variable, 1);
          potential_neg = p_fg->potential(variable, 0);

          drand48_r(this->p_rand_seed, this->p_rand_obj_buf);
          if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg-potential_pos)) < 1.0){
            variable.update(1.0);
          }else{
            variable.update(0.0);
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

