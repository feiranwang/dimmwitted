
#include "factor_graph.pb.h"
#include "io/cmd_parser.h"
#include <numa.h>

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

    long n_var_in_factors;

    bool loading_finalized;
    bool safety_check_passed;

    bool does_change_evid;

    FactorGraph(){
      this->loading_finalized = false;
      this->safety_check_passed = false;
      this->n_var_in_factors = 0;
    }

    double potential(const Variable & variable, const double & proposal){
      return 1.0;
    }

    double potential(const Factor & factor){
      return 1.0;
    }

    void ensure_evids(){
      for(Variable & variable : this->variables){
        if(variable.is_evid){
          variable.current_value = variable.init_value;
        }
      }
    }

    void load(const CmdParser & cmd);

    void finalize_loading();

    void safety_check();

    bool is_usable(){
      return this->loading_finalized && this->safety_check_passed;
    }

  };



  class CompactVariable {
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

    long factor_id_start;
    long factor_id_end;

    void update(const double & new_value){
      this->current_value = new_value;
      this->agg_mean = this->agg_mean + new_value;
      this->n_sample ++;
    }

    CompactVariable(const long & _id, const int & _domain_type, 
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


  /*
    double sum = 0;
    for(const auto & varInFactor : this->variables){
      if(varInFactor.vid == variable.id){
        sum += proposal;
      }else{
        sum += p_fg->variables[varInFactor.vid].current_value;
      }
    }
    if(sum != 0){
      return 1.0;
    }
    return 0.0;
  }
  */




  class CompactFactor{
  public:
    long id;
    long weight_id;
    long var_in_factor_start;
    long var_in_factor_end;
    int func_id; 

    double (*p_func) (const CompactFactor & factor, const CompactVariable * const variables, 
      const VariableInFactor * const var_in_factors, const long & vid, const double & proposal);

    static double _potential_imply(const CompactFactor & factor, const CompactVariable * const variables, 
      const VariableInFactor * const var_in_factors, const long & vid, const double & proposal){

      double sum = 0;
      for(size_t i_vif=factor.var_in_factor_start;i_vif<factor.var_in_factor_end;i_vif++){
        const VariableInFactor & vif = var_in_factors[i_vif];
        if(vif.vid == vid){
          sum += proposal;
        }else{
          sum += variables[vif.vid].current_value;
        }
      }
      if(sum != 0){
        return 1.0;
      }
      return 0.0;

    }

    CompactFactor(const long & _id,
           const int & _weight_id,
           const int & _func_id) :
      p_func(_func_id == 0 ? CompactFactor::_potential_imply :
             NULL)
    {
      this->id = _id;
      this->weight_id = _weight_id;
      this->func_id = _func_id;

      if(p_func == NULL){
        std::cout << "ERROR: The function id " << func_id << " provided is not supported!" << std::endl;
        assert(false);
      }
    }

    double potential(const CompactVariable * const variables, 
      const VariableInFactor * const var_in_factors,
      const long & vid, const double & proposal){
      return p_func(*this, variables, var_in_factors, vid, proposal);
    }

  };


  class CompactFactorGraph{
  public:

    double * const weights;
    bool * const weights_is_fixed;
    CompactVariable * const variables;
    CompactFactor * const factors;
    VariableInFactor * const var_in_factors;
    long * const factor_ids;

    int node_id;

    long n_weights;
    long n_variables;
    long n_factors;
    long n_var_in_factors;

    bool does_change_evid;

    CompactFactorGraph(FactorGraph * const _p_fg, int _node_id):
      n_weights(_p_fg->weights.size()),
      n_variables(_p_fg->variables.size()),
      n_factors(_p_fg->factors.size()),
      n_var_in_factors(_p_fg->n_var_in_factors),
      node_id(_node_id),
      weights((double *) numa_alloc_onnode(_p_fg->weights.size()*sizeof(double),_node_id)),
      weights_is_fixed((bool *) numa_alloc_onnode(_p_fg->weights.size()*sizeof(bool),_node_id)),
      variables((CompactVariable *) numa_alloc_onnode(_p_fg->variables.size()*sizeof(CompactVariable),_node_id)),
      factors((CompactFactor *) numa_alloc_onnode(_p_fg->factors.size()*sizeof(CompactFactor),_node_id)),
      var_in_factors((VariableInFactor *) numa_alloc_onnode(_p_fg->n_var_in_factors*sizeof(VariableInFactor),_node_id)),
      factor_ids((long *) numa_alloc_onnode(_p_fg->n_var_in_factors*sizeof(long),_node_id))
    {
      std::cout << "Start copying factor graph to node " << _node_id << std::endl;

      size_t i_var_in_factors = 0;
      size_t i_factor_ids = 0;

      for(size_t i=0;i<n_weights;i++){
        const Weight & weight = _p_fg->weights[i];
        weights[weight.id] = weight.weight;
        weights_is_fixed[weight.id] = weight.isfixed;
      }

      for(size_t i=0;i<n_variables;i++){
        const Variable & variable = _p_fg->variables[i];
        variables[i] = CompactVariable(variable.id, variable.domain_type, 
             variable.is_evid, variable.lower_bound,
             variable.upper_bound, variable.init_value, 
             variable.current_value);
        variables[i].factor_id_start = i_var_in_factors;
        for(long factor_id : variable.factor_ids){
          factor_ids[i_var_in_factors] = factor_id;
          i_var_in_factors ++;
        }
        variables[i].factor_id_end = i_var_in_factors;
      }      

      for(size_t i=0;i<n_factors;i++){
        const Factor & factor = _p_fg->factors[i];
        factors[i] = CompactFactor(factor.id, factor.weight_id, factor.func_id);
        factors[i].var_in_factor_start = i_factor_ids;
        for(const VariableInFactor & vif : factor.variables){
          var_in_factors[i_factor_ids] = vif;
          i_factor_ids ++;
        }
        factors[i].var_in_factor_end = i_factor_ids;
      }

    }

    double potential(const CompactVariable & variable, const double & proposal){
      double pot = 0.0;
      for(size_t i_fid=variable.factor_id_start;i_fid<variable.factor_id_end;i_fid++){
        pot += weights[factors[factor_ids[i_fid]].weight_id] *
          factors[factor_ids[i_fid]].potential(variables, var_in_factors, variable.id, proposal);
      }
      return pot;
    }

    double potential(CompactFactor & factor){
      return factor.potential(variables, var_in_factors, -1, -1);
    }

    void ensure_evids(){
      for(size_t i=0;i<n_variables;i++){
        if(variables[i].is_evid){
          variables[i].current_value = variables[i].init_value;
        }
      }
    }

    ~CompactFactorGraph(){
      //numa_free(weights, n_weights*sizeof(double));
      //numa_free(variables, n_variables*sizeof(CompactVariable));
      //numa_free(factors, n_compact_factors*sizeof(CompactFactor));
      //numa_free(var_in_factors, n_var_in_factors*sizeof(VariableInFactor));
      //numa_free(factor_ids, n_var_in_factors*sizeof(long));
    }

  };



}

#endif




