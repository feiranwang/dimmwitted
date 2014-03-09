
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

    long nvars;
    long nweights;

    double * const agg_means;
    double * const agg_nsamples; 
    double * const assignments_free;
    double * const assignments_evid;
    double * const weight_values;
    bool * const weights_isfixed;

    InferenceResult(long _nvars, long _nweights):
      assignments_free(new double[_nvars]),
      assignments_evid(new double[_nvars]),
      agg_means(new double[_nvars]),
      agg_nsamples(new double[_nvars]),
      weight_values(new double [_nweights]),
      weights_isfixed(new bool [_nweights]),
      nweights(_nweights),
      nvars(_nvars){}

    void init(Variable * const variables, Weight * const weights){

      for(long t=0;t<nvars;t++){
        const Variable & variable = variables[t];
        assignments_free[variable.id] = variable.assignment_free;
        assignments_evid[variable.id] = variable.assignment_evid;
        agg_means[variable.id] = 0.0;
        agg_nsamples[variable.id] = 0.0;
      }

      for(long t=0;t<nweights;t++){
        const Weight & weight = weights[t];
        weight_values[weight.id] = weight.weight;
        weights_isfixed[weight.id] = weight.isfixed;
      }
    }
  };

  class FactorGraph {
  public:
    
    Variable * const variables;
    Factor * const factors;
    Weight * const weights;

    long n_var;
    long n_factor;
    long n_weight;

    long c_nvar;
    long c_nfactor;
    long c_nweight;

    bool loading_finalized;
    bool safety_check_passed;

    InferenceResult * const infrs ;

    double stepsize;

    FactorGraph(long _n_var, long _n_factor, long _n_weight) : 
      variables(new Variable[_n_var]),
      factors(new Factor[_n_factor]),
      weights(new Weight[_n_weight]),
      n_var(_n_var), n_factor(_n_factor), n_weight(_n_weight),
      infrs(new InferenceResult(_n_var, _n_weight))
    {
      this->loading_finalized = false;
      this->safety_check_passed = false;
      c_nvar = 0;
      c_nfactor = 0;
      c_nweight = 0;
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
      for(int i=0;i<variable.factor_ids.size();i++){
        const long & factor_id = variable.factor_ids[i];
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




