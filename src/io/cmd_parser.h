
#include <string>
#include <iostream>
#include <algorithm>
#include <tclap/CmdLine.h>

namespace dd{

  class CmdParser{
  public:

    TCLAP::ValueArg<std::string> * input_folder;
    TCLAP::ValueArg<std::string> * output_folder;
    TCLAP::ValueArg<int> * n_epochs;
    
    TCLAP::CmdLine * cmd;

    CmdParser(){

      cmd = new TCLAP::CmdLine("DimmWitted", ' ', "0.01");

      input_folder = new TCLAP::ValueArg<std::string>("i","input","Input Folder",true,"","string");
      output_folder = new TCLAP::ValueArg<std::string>("o","output","Output Folder",true,"","string");
      n_epochs = new TCLAP::ValueArg<int>("n","nepochs","Number of Epochs to Execute",true,-1,"int");

      cmd->add(*input_folder);
      cmd->add(*output_folder);
      cmd->add(*n_epochs);

    }

    void parse(int argv, char** argc){
      cmd->parse(argv, argc);
    }

  };

}