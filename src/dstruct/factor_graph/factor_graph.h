
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

  class FactorGraph {
  public:
    std::vector<Variable> variables;
    std::vector<Factor> factors;
    std::vector<Weight> weights;

    bool loading_finalized;
    bool safety_check_passed;

    double stepsize;

    FactorGraph(){
      this->loading_finalized = false;
      this->safety_check_passed = false;
    }

    double update_weight(const Variable & variable){
      for(const long & i_fid:variable.factor_ids){
        const Factor & factor = factors[i_fid];
        if(weights[factor.weight_id].isfixed == false){
          weights[factor.weight_id].weight += 
            stepsize * (this->template potential<false>(factor) 
              - this->template potential<true>(factor));
        }
      }
    }

    template<bool does_change_evid>
    inline double potential(const Factor & factor){
      return factor.template potential<does_change_evid>(
        variables, -1, -1);
    }
  
    template<bool does_change_evid>
    inline void update(Variable & variable, const double & new_value);

    template<bool does_change_evid>
    inline double potential(const Variable & variable, const double & proposal){
      double pot = 0.0;
      for(int i=0;i<variable.n_factors;i++){
        const long & factor_id = variable.factor_ids[i];
        const Factor & factor = factors[factor_id];
        const Weight & weight = weights[factor.weight_id];
        pot += weight.weight*factor.template potential<does_change_evid>(
              variables, variable.id, proposal);
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
    variable.assignment_free = new_value;
  }

  template<>
  inline void FactorGraph::update<false>(Variable & variable, const double & new_value){
    variable.assignment_evid = new_value;
    variable.agg_mean += new_value;
    variable.n_sample ++ ;
  }



}

#endif




