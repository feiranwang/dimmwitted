
#include <iostream>
#include "io/pb_parser.h"
#include "dstruct/factor_graph.h"

void handle_variable(const deepdive::Variable & variable, dd::FactorGraph & fg){
  if(variable.datatype() == deepdive::Variable_VariableDataType_BOOLEAN){
    if(variable.has_initialvalue()){ //TODO: SHOULD NTO CHECK variable.has_initialvalue()
      fg.variables.push_back(
        dd::Variable(variable.id(), DTYPE_BOOLEAN, true, 0, 1, 
          variable.initialvalue(), variable.initialvalue())
      );
      fg.n_var_evid ++;
    }else{
      fg.variables.push_back(
        dd::Variable(variable.id(), DTYPE_BOOLEAN, false, 0, 1, 0, 0)
      );
      fg.n_var_query ++;
    }
    //std::cout << "~~~~" << variable.id() << std::endl;
  }else{
    std::cout << "[ERROR] Only Boolean variables are supported now!" << std::endl;
    assert(false);
  }
}

void handle_factor(const deepdive::Factor & factor, dd::FactorGraph & fg){
  fg.factors.push_back(
    dd::Factor(factor.id(), factor.weightid(), factor.factorfunction())
  );
}

void handle_weight(const deepdive::Weight & weight, dd::FactorGraph & fg){
  fg.weights.push_back(
    dd::Weight(weight.id(), weight.initialvalue(), weight.isfixed())
  );
}

void handle_edge(const deepdive::GraphEdge & edge, dd::FactorGraph & fg){
  fg.factors[edge.factorid()].variables.push_back(
    dd::VariableInFactor(edge.variableid(), edge.position(), edge.ispositive())
  );
  fg.variables[edge.variableid()].factor_ids.push_back(
    edge.factorid()
  );
  fg.n_var_in_factors ++;
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
  assert(n_loaded == this->variables.size());
  std::cout << "LOADED VARIABLES: #" << this->variables.size() << std::endl;
  std::cout << "          QUERY : #" << this->n_var_query << std::endl;
  std::cout << "          EVID  : #" << this->n_var_evid << std::endl;

  n_loaded = dd::stream_load_pb<deepdive::Factor, dd::FactorGraph, handle_factor>(filename_factors, *this);
  assert(n_loaded == this->factors.size());
  std::cout << "LOADED FACTORS: #" << this->factors.size() << std::endl;

  n_loaded = dd::stream_load_pb<deepdive::Weight, dd::FactorGraph, handle_weight>(filename_weights, *this);
  assert(n_loaded == this->weights.size());
  std::cout << "LOADED WEIGHTS: #" << this->weights.size() << std::endl;

  this->finalize_loading();

  n_loaded = dd::stream_load_pb<deepdive::GraphEdge, dd::FactorGraph, handle_edge>(filename_edges, *this);
  std::cout << "LOADED EDGES: #" << n_loaded << std::endl;

  this->safety_check();

  assert(this->is_usable() == true);

}



void dd::FactorGraph::finalize_loading(){
  std::sort(this->variables.begin(), this->variables.end(), idsorter<Variable>());
  std::sort(this->factors.begin(), this->factors.end(), idsorter<Factor>());
  std::sort(this->weights.begin(), this->weights.end(), idsorter<Weight>()); 
  this->loading_finalized = true;
}

void dd::FactorGraph::safety_check(){
  size_t s = variables.size();
  for(size_t i=0;i<s;i++){
    assert(this->variables[i].id == i);
  }
  s = factors.size();
  for(size_t i=0;i<s;i++){
    assert(this->factors[i].id == i);
  }
  s = weights.size();
  for(size_t i=0;i<s;i++){
    assert(this->weights[i].id == i);
  }
  std::cout << "FACTOR GRAPH: Safety Checking Passed..." << std::endl;
  this->safety_check_passed = true;
}

