
#include "dstruct/factor_graph/factor_graph.h"
#include "timer.h"
#include "common.h"

#ifndef _SINGLE_THREAD_SAMPLER_H
#define _SINGLE_THREAD_SAMPLER_H


long ____nn = 0;

namespace dd{

  class SingleThreadSampler{

  public:
    FactorGraph * const p_fg;

    double potential_pos;
    double potential_neg;

    double potential_pos_freeevid;
    double potential_neg_freeevid;

    double proposal_freevid;
    double proposal_fixevid;

    double r;
    unsigned short p_rand_seed[3];
    double * const p_rand_obj_buf;

    std::vector<double> varlen_potential_buffer;

    double sum;
    double acc;
    double multi_proposal;


    SingleThreadSampler(FactorGraph * _p_fg) :
      p_fg (_p_fg), p_rand_obj_buf(new double){
      p_rand_seed[0] = rand();
      p_rand_seed[1] = rand();
      p_rand_seed[2] = rand();
    }

    void sample(const int & i_sharding, const int & n_sharding, double t0){      long nvar = p_fg->n_var;
      long start = ((long)(nvar/n_sharding)+1) * i_sharding;
      long end = ((long)(nvar/n_sharding)+1) * (i_sharding+1);
      end = end > nvar ? nvar : end;
      //for(int i=0;i<100;i++){
      for(long i=start; i<end; i++){
        this->sample_single_variable(i, t0);
      }
      //}
    }

    void sample_sgd(const int & i_sharding, const int & n_sharding){
      ____nn ++;
      long nvar = p_fg->n_var;
      long start = ((long)(nvar/n_sharding)+1) * i_sharding;
      long end = ((long)(nvar/n_sharding)+1) * (i_sharding+1);
      end = end > nvar ? nvar : end;

      //for(int j=0;j<20;j++){
      for(long i=start; i<end; i++){
          this->sample_sgd_single_variable(i);
      }
      //}

      // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

      for(long i=0;i<p_fg->n_factor;i++){
        p_fg->factors[i].is_sampled = false;
      }
      for(long i=0;i<p_fg->n_edge;i++){
        const CompactFactor & f = p_fg->factors_dups[i];
        if (p_fg->factors[f.id].is_sampled == true)
          continue;
        // if(p_fg->factors[f.id].is_sampled == true){
        //   std::cout << "skip f.id = " << f.id << "  edgeid = " << i << std::endl;
        //   continue;
        // }

        long wid1 = p_fg->get_weightid(p_fg->infrs->assignments_evid, f, -1, -1);
        long wid2 = p_fg->get_weightid(p_fg->infrs->assignments_free, f, -1, -1);

        int equal = (wid1 == wid2);

        if(p_fg->infrs->weights_isfixed[wid1] == false){
          p_fg->infrs->weight_values[wid1] += 
              p_fg->stepsize * (p_fg->template potential<false>(f) - equal * p_fg->template potential<true>(f));
        }

        if(wid1 != wid2 && p_fg->infrs->weights_isfixed[wid2] == false){
            p_fg->infrs->weight_values[wid2] += 
              p_fg->stepsize * (equal * p_fg->template potential<false>(f) - p_fg->template potential<true>(f));
        }

        // if (wid1 == 93420 || wid1 == 97209 || wid1 == 98787) {
        //   printf("wid1 = %li, weight = %f\n", wid1, 
        //     p_fg->infrs->weight_values[wid1]);
        // }

        p_fg->factors[f.id].is_sampled = true;

        // std::cout << "WID " << wid1 << " = " << p_fg->infrs->weight_values[wid1] << std::endl;
        // std::cout << "WID " << wid2 << " = " << p_fg->infrs->weight_values[wid2] << std::endl;
        // std::cout << "####" << std::endl;
      }
      // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;


    }

  private:

    void sample_sgd_single_variable(long vid){

      
      
      Variable & variable = p_fg->variables[vid];

      if(variable.is_evid == false){
	return;
      }

      if(variable.domain_type == DTYPE_BOOLEAN){


          if(variable.is_evid == false){
	    //return;
            potential_pos = p_fg->template potential<false>(variable, 1);
            potential_neg = p_fg->template potential<false>(variable, 0);
            *this->p_rand_obj_buf = erand48(this->p_rand_seed);
            if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg-potential_pos)) < 1.0){
              p_fg->template update<false>(variable, 1.0);
            }else{
              p_fg->template update<false>(variable, 0.0);
            }
          }

          potential_pos_freeevid = p_fg->template potential<true>(variable, 1);
          potential_neg_freeevid = p_fg->template potential<true>(variable, 0);

          *this->p_rand_obj_buf = erand48(this->p_rand_seed);
          if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg_freeevid-potential_pos_freeevid)) < 1.0){
            p_fg->template update<true>(variable, 1.0);
          }else{
            p_fg->template update<true>(variable, 0.0);
          }

          this->p_fg->update_weight(variable);
          
      }else if(variable.domain_type == DTYPE_MULTINOMIAL){

        if (variable.is_fixed) return;
        // std::cout << "SAMPLING " << vid << std::endl;

        while(variable.upper_bound >= varlen_potential_buffer.size()){
          varlen_potential_buffer.push_back(0.0);
        }

        if(variable.is_evid == false){
          sum = -100000.0;
          acc = 0.0;
          multi_proposal = -1;
          for(int propose=0;propose <= variable.upper_bound; propose++){
            varlen_potential_buffer[propose] = p_fg->template potential<false>(variable, propose);
            sum = logadd(sum, varlen_potential_buffer[propose]);
          }

          *this->p_rand_obj_buf = erand48(this->p_rand_seed);
          for(int propose=0;propose <= variable.upper_bound; propose++){
            acc += exp(varlen_potential_buffer[propose]-sum);
            if(*this->p_rand_obj_buf <= acc){
              multi_proposal = propose;
              break;
            }
          }
          assert(multi_proposal != -1.0);
          p_fg->template update<false>(variable, multi_proposal);
        }

        sum = -1000000000000.0;
        acc = 0.0;
        multi_proposal = -1;
        for(int propose=0;propose <= variable.upper_bound; propose++){
          varlen_potential_buffer[propose] = p_fg->template potential<true>(variable, propose);
          //printf("potential2 = %f\n", varlen_potential_buffer[propose]);
          sum = logadd(sum, varlen_potential_buffer[propose]);
        }

        //printf("sum = %f\n", sum);

        *this->p_rand_obj_buf = erand48(this->p_rand_seed);
        for(int propose=0;propose <= variable.upper_bound; propose++){
          acc += exp(varlen_potential_buffer[propose]-sum);
          
          // printf("~~~~~potential2 = %f   sum = %f   acc = %f    rand = %f\n", varlen_potential_buffer[propose], sum, acc, *this->p_rand_obj_buf);

          if(*this->p_rand_obj_buf <= acc){
            multi_proposal = propose;
            break;
          }
        }

        /*
        if(multi_proposal == -1.0){
          acc = 0.0;
          for(int propose=0;propose <= variable.upper_bound; propose++){
            acc += exp(varlen_potential_buffer[propose]-sum);
            printf("~~~~~potential2 = %f   sum = %f   acc = %f\n", varlen_potential_buffer[propose], sum, acc);
          }
        }
        */
        
        assert(multi_proposal != -1.0);
        p_fg->template update<true>(variable, multi_proposal);

        //std::cout << "~~~~" << ____nn << std::endl;

        //if(____nn % 100 == 0){
          //if(variable.id == 1){
          //this->p_fg->update_weight(variable);
          
          //}
        //}

      }else{
        std::cout << "[ERROR] Only Boolean and Multinomial variables are supported now!" << std::endl;
        assert(false);
      }
      
    }

    void sample_single_variable(long vid, double t0){

      Variable & variable = this->p_fg->variables[vid];

      if(variable.domain_type == DTYPE_BOOLEAN){

        if(variable.is_evid == false){

          potential_pos = p_fg->template potential<false>(variable, 1);
          potential_neg = p_fg->template potential<false>(variable, 0);

          *this->p_rand_obj_buf = erand48(this->p_rand_seed);
          if((*this->p_rand_obj_buf) * (1.0 + exp(potential_neg-potential_pos)) < 1.0){
            p_fg->template update<false>(variable, 1.0);
          }else{
            p_fg->template update<false>(variable, 0.0);
          }

        }

      }else if(variable.domain_type == DTYPE_MULTINOMIAL){

        while(variable.upper_bound >= varlen_potential_buffer.size()){
          varlen_potential_buffer.push_back(0.0);
        }

        if(variable.is_evid == false){
          sum = -100000.0;
          acc = 0.0;
          multi_proposal = -1;
          for(int propose=0;propose <= variable.upper_bound; propose++){
            varlen_potential_buffer[propose] = 1/t0 * p_fg->template potential<false>(variable, propose);
            sum = logadd(sum, varlen_potential_buffer[propose]);
          }

          *this->p_rand_obj_buf = erand48(this->p_rand_seed);
          for(int propose=0;propose <= variable.upper_bound; propose++){
            acc += exp(varlen_potential_buffer[propose]-sum);
            if(*this->p_rand_obj_buf <= acc){
              multi_proposal = propose;
              break;
            }
          }
          assert(multi_proposal != -1.0);
          p_fg->template update<false>(variable, multi_proposal);
        }

      }else{
        std::cout << "[ERROR] Only Boolean and Multinomial variables are supported now!" << std::endl;
        assert(false);
      }

    }

  };

}


#endif

