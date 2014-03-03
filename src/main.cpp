
#include <iostream>

#include "io/cmd_parser.h"

int main(int argv, char** argc){

  dd::CmdParser cmd_parser;
  cmd_parser.parse(argv, argc);

  std::cout << cmd_parser.input_folder->getValue() << std::endl;


}