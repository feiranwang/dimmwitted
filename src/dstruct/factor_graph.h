
#include "factor_graph.pb.h"
#include "io/cmd_parser.h"
#include <numa.h>
#include <xmmintrin.h>

#ifndef _FACTOR_GRAPH_H_
#define _FACTOR_GRAPH_H_

#define DTYPE_BOOLEAN     0x00
#define DTYPE_REAL        0x01
#define DTYPE_MULTINOMIAL 0x04

/*
enum _mm_hint
{
  _MM_HINT_T0 = 3,
  _MM_HINT_T1 = 2,
  _MM_HINT_T2 = 1,
  _MM_HINT_NTA = 0
};
*/

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

    long n_var_query;
    long n_var_evid;

    bool loading_finalized;
    bool safety_check_passed;

    bool does_change_evid;

    FactorGraph(){
      this->loading_finalized = false;
      this->safety_check_passed = false;
      this->n_var_in_factors = 0;
      this->n_var_query = 0;
      this->n_var_evid = 0;
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


  class CompactFactor{
  public:
    long id;
    long weight_id;
    
    long var_in_factor_start;
    long var_in_factor_end;

    VariableInFactor * vifs;
    int n_vifs;

    int func_id; 

    double (*p_func) (const CompactFactor & factor, const double * const variable_values, 
      const VariableInFactor * const var_in_factors, const long & vid, const double & proposal);

    static double _potential_imply(const CompactFactor & factor, const double * const variable_values, 
      const VariableInFactor * const var_in_factors, const long & vid, const double & proposal){

      double sum = 0.0;

      for(size_t i_vif=0;i_vif<factor.n_vifs;i_vif++){
        
        const VariableInFactor & vif = factor.vifs[i_vif];

        if(vif.n_position == factor.n_vifs - 1){
          if(vif.vid == vid){
            sum += (vif.is_positive == true ? proposal : 1-proposal);
          }else{
            sum += (vif.is_positive == true ? variable_values[vif.vid] : 1-variable_values[vif.vid]);
          }
        }else{
          if(vif.vid == vid){
            sum += (vif.is_positive == false ? proposal : 1-proposal);
          }else{
            sum += (vif.is_positive == false ? variable_values[vif.vid] : 1-variable_values[vif.vid]);
          }
        }
      }

      if(sum != 0){
        return 1.0;
      }else{
        return 0.0;
      }

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

    CompactFactor(const long & _id,
           const int & _weight_id,
           const int & _func_id,
           VariableInFactor * _vifs,
           const int & _n_vifs) :
      p_func(_func_id == 0 ? CompactFactor::_potential_imply :NULL),
      vifs(_vifs),
      n_vifs(_n_vifs)
    {
      this->id = _id;
      this->weight_id = _weight_id;
      this->func_id = _func_id;

      if(p_func == NULL){
        std::cout << "ERROR: The function id " << func_id << " provided is not supported!" << std::endl;
        assert(false);
      }
    }



    double potential(const double * const variable_values, 
      const VariableInFactor * const var_in_factors,
      const long & vid, const double & proposal) const{
      return p_func(*this, variable_values, var_in_factors, vid, proposal);
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

    CompactFactor * factors;
    int n_factors;

    //long factor_id_start;
    //long factor_id_end;

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




  class CompactFactorGraph_Mutable{
  public:

    double * const weights;
    double * const variable_assignments_free;
    double * const variable_assignments_evid;

    double * const agg_means;
    double * const agg_nsamples;

    double * const factor_tallies_free;
    double * const factor_tallies_evid;

    long n_weights;
    long n_variables;

    CompactFactorGraph_Mutable(FactorGraph * const _p_fg, int _node_id):
      n_weights(_p_fg->weights.size()),
      n_variables(_p_fg->variables.size()),
      weights((double *) numa_alloc_onnode(_p_fg->weights.size()*sizeof(double),_node_id)),
      variable_assignments_free((double *) numa_alloc_onnode(_p_fg->variables.size()*sizeof(double),_node_id)),
      variable_assignments_evid((double *) numa_alloc_onnode(_p_fg->variables.size()*sizeof(double),_node_id)),
      agg_means((double *) numa_alloc_onnode(_p_fg->variables.size()*sizeof(double),_node_id)),
      agg_nsamples((double *) numa_alloc_onnode(_p_fg->variables.size()*sizeof(double),_node_id)),
      factor_tallies_free((double *) numa_alloc_onnode(_p_fg->factors.size()*sizeof(double),_node_id)),
      factor_tallies_evid((double *) numa_alloc_onnode(_p_fg->factors.size()*sizeof(double),_node_id))
    {

      std::cout << "Start copying factor graph (mutable part) to node " << _node_id << std::endl;

      for(size_t i=0;i<n_weights;i++){
        const Weight & weight = _p_fg->weights[i];
        weights[weight.id] = weight.weight;
      }

      for(size_t i=0;i<n_variables;i++){
        const Variable & variable = _p_fg->variables[i];
        variable_assignments_free[i] = variable.init_value;
        variable_assignments_evid[i] = variable.init_value;
        agg_means[i] = 0;
        agg_nsamples[i] = 0;
      }      
    }
  };

  class CompactFactorGraph_Immutable{
  public:

    //double * const weights;
    bool * const weights_is_fixed;
    CompactVariable * const variables;
    CompactFactor * const factors;
    VariableInFactor * const var_in_factors;
    long * const factor_ids;
    CompactFactor * const factor_ids_copies;

    int node_id;

    long n_weights;
    long n_variables;
    long n_factors;
    long n_var_in_factors;

    CompactFactorGraph_Immutable(FactorGraph * const _p_fg, int _node_id):
      n_weights(_p_fg->weights.size()),
      n_variables(_p_fg->variables.size()),
      n_factors(_p_fg->factors.size()),
      n_var_in_factors(_p_fg->n_var_in_factors),
      node_id(_node_id),
      //weights((double *) numa_alloc_onnode(_p_fg->weights.size()*sizeof(double),_node_id)),
      weights_is_fixed((bool *) numa_alloc_onnode(_p_fg->weights.size()*sizeof(bool),_node_id)),
      variables((CompactVariable *) numa_alloc_onnode(_p_fg->variables.size()*sizeof(CompactVariable),_node_id)),
      factors((CompactFactor *) numa_alloc_onnode(_p_fg->factors.size()*sizeof(CompactFactor),_node_id)),
      var_in_factors((VariableInFactor *) numa_alloc_onnode(_p_fg->n_var_in_factors*sizeof(VariableInFactor),_node_id)),
      factor_ids((long *) numa_alloc_onnode(_p_fg->n_var_in_factors*sizeof(long),_node_id)),
      factor_ids_copies((CompactFactor *) numa_alloc_onnode(_p_fg->n_var_in_factors*sizeof(CompactFactor),_node_id))
    {
      std::cout << "Start copying factor graph (immutable part) to node " << _node_id << std::endl;

      size_t i_var_in_factors = 0;
      size_t i_factor_ids = 0;

      for(size_t i=0;i<n_weights;i++){
        const Weight & weight = _p_fg->weights[i];
        //weights[weight.id] = weight.weight;
        weights_is_fixed[weight.id] = weight.isfixed;
      }

      long n_factor;
      for(size_t i=0;i<n_variables;i++){
        const Variable & variable = _p_fg->variables[i];
        variables[i] = CompactVariable(variable.id, variable.domain_type, 
             variable.is_evid, variable.lower_bound,
             variable.upper_bound, variable.init_value, 
             variable.current_value);
        //variables[i].factor_id_start = i_var_in_factors;
        n_factor = i_var_in_factors;
        variables[i].factors = &factor_ids_copies[i_var_in_factors];
        for(long factor_id : variable.factor_ids){
          factor_ids[i_var_in_factors] = factor_id;
          i_var_in_factors ++;
        }
        //variables[i].factor_id_end = i_var_in_factors;
        variables[i].n_factors = i_var_in_factors-n_factor;
      }      

      for(size_t i=0;i<n_factors;i++){
        const Factor & factor = _p_fg->factors[i];
        factors[i] = CompactFactor(factor.id, factor.weight_id, factor.func_id);
        factors[i].var_in_factor_start = i_factor_ids;
        factors[i].vifs = &var_in_factors[i_factor_ids];
        for(const VariableInFactor & vif : factor.variables){
          var_in_factors[i_factor_ids] = vif;
          i_factor_ids ++;
        }
        factors[i].var_in_factor_end = i_factor_ids;
        factors[i].n_vifs = factors[i].var_in_factor_end - factors[i].var_in_factor_start;
      }

      for(size_t i=0;i<n_var_in_factors;i++){
        const long & factor_id = factor_ids[i];
        factor_ids_copies[i] = CompactFactor(factors[factor_id].id, factors[factor_id].weight_id, 
        factors[factor_id].func_id, &var_in_factors[factors[factor_id].var_in_factor_start],
        factors[factor_id].var_in_factor_end-factors[factor_id].var_in_factor_start);
        //factor_ids_copies[i].var_in_factor_start = factors[factor_id].var_in_factor_start;
        //factor_ids_copies[i].var_in_factor_end = factors[factor_id].var_in_factor_end;
        //std::cout << (factor_ids_copies[i].var_in_factor_end-factor_ids_copies[i].var_in_factor_start) << std::endl;
      }

    }
  };

  class CompactFactorGraph{
  public:

    long n_weights;
    long n_variables;
    long n_factors;
    long n_var_in_factors;

    CompactFactorGraph_Immutable * const fg_immutable;
    CompactFactorGraph_Mutable * const fg_mutable;

    const bool * const weights_is_fixed;
    const CompactVariable * const variables;
    const CompactFactor * const factors;
    const VariableInFactor * const var_in_factors;
    const long * const factor_ids;

    double * const weights;

    double * const variable_assignments_free;
    double * const variable_assignments_evid;
    double * const agg_means;
    double * const agg_nsamples;
    double * const factor_tallies_free;
    double * const factor_tallies_evid;

    double stepsize;

    CompactFactorGraph(CompactFactorGraph_Immutable * const _fg_immutable,
      CompactFactorGraph_Mutable * const _fg_mutable):
      fg_immutable(_fg_immutable), fg_mutable(_fg_mutable),
      n_weights(_fg_immutable->n_weights),
      n_variables(_fg_immutable->n_variables),
      n_factors(_fg_immutable->n_factors),
      n_var_in_factors(_fg_immutable->n_var_in_factors),
      weights_is_fixed(_fg_immutable->weights_is_fixed),
      variables(_fg_immutable->variables),
      factors(_fg_immutable->factors),
      var_in_factors(_fg_immutable->var_in_factors),
      factor_ids(_fg_immutable->factor_ids),
      weights(_fg_mutable->weights),
      variable_assignments_free(_fg_mutable->variable_assignments_free),
      variable_assignments_evid(_fg_mutable->variable_assignments_evid),
      agg_means(_fg_mutable->agg_means),
      agg_nsamples(_fg_mutable->agg_nsamples),
      factor_tallies_free(_fg_mutable->factor_tallies_free),
      factor_tallies_evid(_fg_mutable->factor_tallies_evid)
      {

      }

    double potential(const CompactVariable & variable, const double & proposal, const bool & does_change_evid){
      double pot = 0.0;
      //for(size_t i_fid=variable.factor_id_start;i_fid<variable.factor_id_end;i_fid++){

      for(size_t i_fid=0;i_fid<variable.n_factors;i_fid++){

        //_mm_prefetch((void*) &fg_immutable->factor_ids_copies[i_fid+1], _MM_HINT_T1);
        //_mm_prefetch((void*) &weights[fg_immutable->factor_ids_copies[i_fid+1].weight_id], _MM_HINT_T1);


        const CompactFactor & factor = variable.factors[i_fid];
        const double & weight = weights[factor.weight_id];

        if(does_change_evid){ // TODO: change this to template
          pot += weight*factor.potential(variable_assignments_free, 
            var_in_factors, variable.id, proposal);
        }else{
          pot += weight * factor.potential(variable_assignments_evid, 
            var_in_factors, variable.id, proposal);
        }
      }
      return pot;
    }

    double update_weight(const CompactVariable & variable){
      for(size_t i_fid=0;i_fid<variable.n_factors;i_fid++){
        const CompactFactor & factor = variable.factors[i_fid];
        if(weights_is_fixed[factor.weight_id] == false){
          double & weight = weights[factor.weight_id];
          weight += stepsize * (this->potential(factor, false) - this->potential(factor, true));
        }
      }
    }


    double potential(const CompactFactor & factor, const bool & does_change_evid){
      if(does_change_evid){
        return factor.potential(fg_mutable->variable_assignments_free, fg_immutable->var_in_factors, -1, -1);
      }else{
        return factor.potential(fg_mutable->variable_assignments_evid, fg_immutable->var_in_factors, -1, -1);
      }  
    }

    void update(const CompactVariable & variable, const double & new_value, const bool & does_change_evid){
      if(does_change_evid){
        // it does not make sense to tally varialbes when we can change
        // evidences
        fg_mutable->variable_assignments_free[variable.id] = new_value;
        //fg_mutable->agg_means[variable.id] += new_value;
        //fg_mutable->agg_nsamples[variable.id] ++;
      }else{
        fg_mutable->variable_assignments_evid[variable.id] = new_value;
        fg_mutable->agg_means[variable.id] += new_value;
        fg_mutable->agg_nsamples[variable.id] ++;
      }
    }

  };

}

#endif




