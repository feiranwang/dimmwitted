
#include <iostream>
#include "io/pb_parser.h"
#include "dstruct/factor_graph.h"

void handle_variable(const deepdive::Variable & variable, dd::FactorGraph & fg){
  if(variable.datatype() == deepdive::Variable_VariableDataType_BOOLEAN){
    if(variable.has_initialvalue()){
      fg.variables.push_back(
        dd::Variable(variable.id(), DTYPE_BOOLEAN, true, 0, 1, 
          variable.initialvalue(), variable.initialvalue())
      );
    }else{
      fg.variables.push_back(
        dd::Variable(variable.id(), DTYPE_BOOLEAN, false, 0, 1, 0, 0)
      );
    }
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
}

void dd::FactorGraph::load(const CmdParser & cmd){

  std::string input_folder = cmd.input_folder->getValue();

  std::string filename_edges = input_folder + "/graph.edges.pb";
  std::string filename_factors = input_folder + "/graph.factors.pb";
  std::string filename_variables = input_folder + "/graph.variables.pb";
  std::string filename_weights = input_folder + "/graph.weights.pb";


  long n_loaded = dd::stream_load_pb<deepdive::Variable, dd::FactorGraph, handle_variable>(filename_variables, *this);
  assert(n_loaded == this->variables.size());
  std::cout << "LOADED VARIABLES: #" << this->variables.size() << std::endl;

  n_loaded = dd::stream_load_pb<deepdive::Factor, dd::FactorGraph, handle_factor>(filename_factors, *this);
  assert(n_loaded == this->factors.size());
  std::cout << "LOADED FACTORS: #" << this->factors.size() << std::endl;

  n_loaded = dd::stream_load_pb<deepdive::Weight, dd::FactorGraph, handle_weight>(filename_weights, *this);
  assert(n_loaded == this->weights.size());
  std::cout << "LOADED WEIGHTS: #" << this->weights.size() << std::endl;

  //this->finalize_loading();

  n_loaded = dd::stream_load_pb<deepdive::GraphEdge, dd::FactorGraph, handle_edge>(filename_edges, *this);
  std::cout << "LOADED EDGES: #" << n_loaded << std::endl;

  //this->safety_check();

  //assert(this->is_usable() == true);

}