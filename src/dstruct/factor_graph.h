
#include "factor_graph.pb.h"
#include <io/cmd_parser.h>

#ifndef _FACTOR_GRAPH_H_
#define _FACTOR_GRAPH_H_

#define DTYPE_BOOLEAN     0x00
#define DTYPE_REAL        0x01
#define DTYPE_MULTINOMIAL 0x04

namespace dd{

  class Variable {
  public:
    long id;
    int domain_type;
    bool is_evid;
    double lower_bound;
    double upper_bound;
    double init_value;
    double current_value;
    double n_sample;
    double agg_mean;
    std::vector<long> factor_ids;

    void update(const double & new_value){
      this->current_value = new_value;
      this->agg_mean = this->agg_mean + new_value;
      this->n_sample ++;
    }

    Variable(const long & _id, const int & _domain_type, 
             const bool & _is_evid, const double & _lower_bound,
             const double & _upper_bound, const double & _init_value, 
             const double & _current_value){

      this->id = _id;
      this->domain_type = _domain_type;
      this->is_evid = _is_evid;
      this->lower_bound = _lower_bound;
      this->upper_bound = _upper_bound;
      this->init_value = _init_value;
      this->current_value = _current_value;

      this->n_sample = 0.0;
      this->agg_mean = 0.0;

    }
  };

  class VariableInFactor {
  public:
    long vid;
    int n_position;
    bool is_positive;

    VariableInFactor(const long & _vid, const int & _n_position, 
                     const bool & _is_positive){
      this->vid = _vid;
      this->n_position = _n_position;
      this->is_positive = _is_positive;
    }
  };

  class Factor {
  public:
    long id;
    long weight_id;
    std::vector<VariableInFactor> variables;
    int func_id; 

    Factor(const long & _id,
           const int & _weight_id,
           const int & _func_id){
      this->id = _id;
      this->weight_id = _weight_id;
      this->func_id = _func_id;
    }
  };

  class Weight {
  public:
    long id;
    double weight;
    bool isfixed;

    Weight(const long & _id,
           const double & _weight,
           const bool & _isfixed){
      this->id = _id;
      this->weight = _weight;
      this->isfixed = _isfixed;
    }
  };  

  class FactorGraph {
  public:
    std::vector<Variable> variables;
    std::vector<Factor> factors;
    std::vector<Weight> weights;

    bool loading_finalized;
    bool safety_check_passed;

    FactorGraph(){
      this->loading_finalized = false;
      this->safety_check_passed = false;
    }

    void load(const CmdParser & cmd);

    void finalize_loading();

    void safety_check();

    bool is_usable(){
      return this->loading_finalized && this->safety_check_passed;
    }

  };

}

#endif




