
#include <iostream>

#include "io/cmd_parser.h"
#include "io/pb_parser.h"

#include "app/gibbs/gibbs_sampling.h"
#include "dstruct/factor_graph.h"

int main(int argv, char** argc){

  dd::CmdParser cmd_parser;
  cmd_parser.parse(argv, argc);

  std::cout << cmd_parser.input_folder->getValue() << std::endl;

  dd::FactorGraph fg;
  fg.load(cmd_parser);

  //dd::CompactFactorGraph(&fg, 0);

  dd::GibbsSampling gibbs(&fg, &cmd_parser);

  gibbs.inference(50);
  gibbs.dump();

  //gibbs.learn(1000, 1, 0.00001);
  //gibbs.dump_weights();



}