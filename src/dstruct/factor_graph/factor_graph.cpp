
#include <iostream>
#include "io/pb_parser.h"
#include "dstruct/factor_graph/factor_graph.h"

void handle_metadata(const deepdive::FactorGraph & factorgraph, dd::FactorGraph & fg){
  std::cout << factorgraph.numweights() << std::endl;
  std::cout << factorgraph.numvariables() << std::endl;
  std::cout << factorgraph.numfactors() << std::endl;
  std::cout << factorgraph.numedges() << std::endl;
}

void handle_variable(const deepdive::Variable & variable, dd::FactorGraph & fg){

  if(variable.datatype() == deepdive::Variable_VariableDataType_BOOLEAN){
    if(variable.isevidence()){ //TODO: SHOULD NTO CHECK variable.has_initialvalue()
      fg.variables[fg.c_nvar] = dd::Variable(variable.id(), DTYPE_BOOLEAN, true, 0, 1, 
        variable.initialvalue(), variable.initialvalue(), variable.edgecount());
      fg.c_nvar ++;
      fg.n_evid ++;
    }else{
      fg.variables[fg.c_nvar] = dd::Variable(variable.id(), DTYPE_BOOLEAN, false, 0, 1, 0, 0, 
        variable.edgecount());
      fg.c_nvar ++;
      fg.n_query ++;
    }
    //std::cout << "~~~~" << variable.id() << std::endl;
  }else if(variable.datatype() == deepdive::Variable_VariableDataType_MULTINOMIAL){
    if(variable.isevidence()){ //TODO: SHOULD NTO CHECK variable.has_initialvalue()
      fg.variables[fg.c_nvar] = dd::Variable(variable.id(), DTYPE_MULTINOMIAL, true, 0, variable.cardinality()-1, 
        variable.initialvalue(), variable.initialvalue(), variable.edgecount());
      fg.c_nvar ++;
      fg.n_query ++;
    }else{
      fg.variables[fg.c_nvar] = dd::Variable(variable.id(), DTYPE_MULTINOMIAL, false, 0, variable.cardinality()-1, 0, 0, 
        variable.edgecount());
      fg.c_nvar ++;
      fg.n_query ++;
    }
    //std::cout << "~~~~" << variable.id() << std::endl;
  }else{    
    std::cout << "[ERROR] Only Boolean and Multinomial variables are supported now!" << std::endl;
    assert(false);
  }


}

void handle_factor(const deepdive::Factor & factor, dd::FactorGraph & fg){
  //fg.factors.push_back(
  //  dd::Factor(factor.id(), factor.weightid(), factor.factorfunction(), factor.edgecount())
  //);
  fg.factors[fg.c_nfactor] = dd::Factor(factor.id(), factor.weightid(), factor.factorfunction(), factor.edgecount());
  fg.c_nfactor ++;
}

void handle_weight(const deepdive::Weight & weight, dd::FactorGraph & fg){
  //fg.weights.push_back(
  //  dd::Weight(weight.id(), weight.initialvalue(), weight.isfixed())
  //);
  fg.weights[fg.c_nweight] = dd::Weight(weight.id(), weight.initialvalue(), weight.isfixed());
  fg.c_nweight ++;
}

void handle_edge(const deepdive::GraphEdge & edge, dd::FactorGraph & fg){

  if(!edge.has_equalpredicate()){
    fg.factors[edge.factorid()].tmp_variables.push_back(
      dd::VariableInFactor(edge.variableid(), edge.position(), edge.ispositive())
    );
  }else{
    fg.factors[edge.factorid()].tmp_variables.push_back(
      dd::VariableInFactor(edge.variableid(), edge.position(), edge.ispositive(), edge.equalpredicate())
    );
  }

  
  fg.variables[edge.variableid()].tmp_factor_ids.push_back(
    edge.factorid()
  );
}


template <class OBJTOSORT>
class idsorter : public std::binary_function<OBJTOSORT, OBJTOSORT, bool>{
public:
  inline bool operator()(const OBJTOSORT & left, const OBJTOSORT & right){
    return left.id < right.id;  
  }
};

void dd::FactorGraph::load(const CmdParser & cmd){

  std::string weight_file = cmd.weight_file->getValue();
  std::string variable_file = cmd.variable_file->getValue();
  std::string factor_file = cmd.factor_file->getValue();
  std::string edge_file = cmd.edge_file->getValue();

  std::string filename_edges = edge_file;
  std::string filename_factors = factor_file;
  std::string filename_variables = variable_file;
  std::string filename_weights = weight_file;

  long n_loaded = dd::stream_load_pb<deepdive::Variable, dd::FactorGraph, handle_variable>(filename_variables, *this);
  assert(n_loaded == n_var);
  std::cout << "LOADED VARIABLES: #" << n_loaded << std::endl;
  std::cout << "         N_QUERY: #" << n_query << std::endl;
  std::cout << "         N_EVID : #" << n_evid << std::endl;  
  n_loaded = dd::stream_load_pb<deepdive::Factor, dd::FactorGraph, handle_factor>(filename_factors, *this);
  assert(n_loaded == n_factor);
  std::cout << "LOADED FACTORS: #" << n_loaded << std::endl;

  n_loaded = dd::stream_load_pb<deepdive::Weight, dd::FactorGraph, handle_weight>(filename_weights, *this);
  assert(n_loaded == n_weight);
  std::cout << "LOADED WEIGHTS: #" << n_loaded << std::endl;

  this->finalize_loading();

  n_loaded = dd::stream_load_pb<deepdive::GraphEdge, dd::FactorGraph, handle_edge>(filename_edges, *this);
  std::cout << "LOADED EDGES: #" << n_loaded << std::endl;

  this->safety_check();

  assert(this->is_usable() == true);

}

void dd::FactorGraph::finalize_loading(){
  std::sort(&variables[0], &variables[n_var], idsorter<Variable>());
  std::sort(&factors[0], &factors[n_factor], idsorter<Factor>());
  std::sort(&weights[0], &weights[n_weight], idsorter<Weight>()); 
  this->loading_finalized = true;
  infrs->init(variables, weights);
}

void dd::FactorGraph::safety_check(){

  /*
  c_edge = 0;
  for(long i=0;i<n_var;i++){
    Variable & variable = variables[i];
    variable.n_start_i_factors = c_edge;
    for(const long & fid : variable.tmp_factor_ids){
      factor_ids[c_edge] = fid;
      factors_dups[c_edge].id = factors[fid].id;
      factors_dups[c_edge].func_id = factors[fid].func_id;
      factors_dups[c_edge].n_variables = factors[fid].n_variables;
      factors_dups[c_edge].n_start_i_vif = factors[fid].n_start_i_vif;
      factors_dups_weightids[c_edge] = factors[fid].weight_id;
      c_edge ++;
    }
  }*/

  c_edge = 0;
  for(long i=0;i<n_factor;i++){
    Factor & factor = factors[i];
    factor.n_start_i_vif = c_edge;
    for(const VariableInFactor & vif : factor.tmp_variables){
      vifs[c_edge] = vif;
      c_edge ++;
    }
  }

  c_edge = 0;
  for(long i=0;i<n_var;i++){
    Variable & variable = variables[i];
    variable.n_start_i_factors = c_edge;
    for(const long & fid : variable.tmp_factor_ids){
      factor_ids[c_edge] = fid;
      factors_dups[c_edge].id = factors[fid].id;
      factors_dups[c_edge].func_id = factors[fid].func_id;
      factors_dups[c_edge].n_variables = factors[fid].n_variables;
      factors_dups[c_edge].n_start_i_vif = factors[fid].n_start_i_vif;
      factors_dups_weightids[c_edge] = factors[fid].weight_id;
      c_edge ++;
    }
  }

  long s = n_var;
  for(long i=0;i<s;i++){
    //std::cout << this->variables[i].id << "    " << i << std::endl;
    assert(this->variables[i].id == i);
  }
  s = n_factor;
  for(long i=0;i<s;i++){
    assert(this->factors[i].id == i);
  }
  s = n_weight;
  for(long i=0;i<s;i++){
    assert(this->weights[i].id == i);
  }
  std::cout << "FACTOR GRAPH: Safety Checking Passed..." << std::endl;
  this->safety_check_passed = true;
}






