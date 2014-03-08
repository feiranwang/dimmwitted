
#include "dstruct/factor_graph/factor_graph.pb.h"
#include "io/cmd_parser.h"
#include "common.h"

#include "dstruct/factor_graph/variable.h"
#include "dstruct/factor_graph/factor.h"
#include "dstruct/factor_graph/weight.h"

#include <xmmintrin.h>

#ifndef _FACTOR_GRAPH_H_
#define _FACTOR_GRAPH_H_

#define DTYPE_BOOLEAN     0x00
#define DTYPE_REAL        0x01
#define DTYPE_MULTINOMIAL 0x04

namespace dd{

  class InferenceResult{
  public:

    int nvars;
    int nweights;

    double * const agg_means;
    double * const agg_nsamples; 
    double * const assignments_free;
    double * const assignments_evid;
    double * const weight_values;
    bool * const weights_isfixed;

    InferenceResult(
      std::vector<Variable> variables,
      std::vector<Weight> weights,
      size_t _nvars, size_t _nweights):
      assignments_free(new double[_nvars]),
      assignments_evid(new double[_nvars]),
      agg_means(new double[_nvars]),
      agg_nsamples(new double[_nvars]),
      weight_values(new double [_nweights]),
      weights_isfixed(new bool [_nweights]){

      for(const Variable & variable : variables){
        assignments_free[variable.id] = variable.assignment_free;
        assignments_evid[variable.id] = variable.assignment_evid;
        agg_means[variable.id] = 0.0;
        agg_nsamples[variable.id] = 0.0;
      }

      for(const Weight & weight: weights){
        weight_values[weight.id] = weight.weight;
        weights_isfixed[weight.id] = weight.isfixed;
      }
    }
  };


  class FactorGraph {
  public:
    std::vector<Variable> variables;
    std::vector<Factor> factors;
    std::vector<Weight> weights;

    bool loading_finalized;
    bool safety_check_passed;

    InferenceResult * infrs;

    double stepsize;

    FactorGraph(){
      this->loading_finalized = false;
      this->safety_check_passed = false;
    }

    double update_weight(const Variable & variable){
      for(const long & i_fid:variable.factor_ids){
        const Factor & factor = factors[i_fid];
        if(infrs->weights_isfixed[factor.weight_id] == false){
          infrs->weight_values[factor.weight_id] += 
            stepsize * (this->template potential<false>(factor) 
              - this->template potential<true>(factor));
        }
      }
    }

    template<bool does_change_evid>
    inline double potential(const Factor & factor){
      if(does_change_evid == true){
        return factor.potential(infrs->assignments_free, -1, -1);
      }else{
        return factor.potential(infrs->assignments_evid, -1, -1);
      }
    }
  
    template<bool does_change_evid>
    inline void update(Variable & variable, const double & new_value);

    template<bool does_change_evid>
    inline double potential(const Variable & variable, const double & proposal){
      double pot = 0.0;
      for(const long & factor_id : variable.factor_ids){
        const Factor & factor = factors[factor_id];
        const double & weight = infrs->weight_values[factor.weight_id];
        if(does_change_evid == true){
          pot += weight*factor.potential(
              infrs->assignments_free, variable.id, proposal);
        }else{
          pot += weight*factor.potential(
              infrs->assignments_evid, variable.id, proposal);
        }
      }
      return pot;
    }

    void load(const CmdParser & cmd);

    void finalize_loading();

    void safety_check();

    bool is_usable(){
      return this->loading_finalized && this->safety_check_passed;
    }

  };

  template<>
  inline void FactorGraph::update<true>(Variable & variable, const double & new_value){
    infrs->assignments_free[variable.id] = new_value;
  }

  template<>
  inline void FactorGraph::update<false>(Variable & variable, const double & new_value){
    infrs->assignments_evid[variable.id] = new_value;
    infrs->agg_means[variable.id] += new_value;
    infrs->agg_nsamples[variable.id]  ++ ;
  }



}

#endif




