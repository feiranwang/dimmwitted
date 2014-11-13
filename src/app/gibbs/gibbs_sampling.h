
#include <iostream>
#include "io/cmd_parser.h"
#include "dstruct/factor_graph/factor_graph.h"

#ifndef _GIBBS_SAMPLING_H_
#define _GIBBS_SAMPLING_H_

namespace dd{
  class GibbsSampling{
  public:

    FactorGraph * const p_fg;

    CmdParser * const p_cmd_parser;

    int n_numa_nodes;

    int n_thread_per_numa;

    std::vector<FactorGraph> factorgraphs;

    GibbsSampling(FactorGraph * const _p_fg, CmdParser * const _p_cmd_parser, int n_datacopy, bool is_quiet) 
      : p_fg(_p_fg), p_cmd_parser(_p_cmd_parser){
      prepare(n_datacopy, is_quiet);
    }

    void prepare(int n_datacopy, bool is_quiet);

    void inference(const int & n_epoch, bool is_quiet);

    void dump(bool is_quiet);

    void dump_weights(bool is_quiet);

    void learn(const int & n_epoch, const int & n_sample_per_epoch, 
      const double & stepsize, const double & decay, bool is_quiet);

  };
}




#endif