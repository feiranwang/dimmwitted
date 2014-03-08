dimmwitted
==========

# Installation

First, install dependencies

    make dep

Then

    make

This will use whatever in you $(CXX) variable to compile. We assume that you have > g++4.7.2 or clang++-4.2. To specify a compiler to use, type in something like

    CXX=/dfs/rulk/0/czhang/software/gcc/bin/g++ make

To test

    make gibbs

This should output things like

    ./dw gibbs -e data/ -o data/ -i 100 -l 100 -s 10 --alpha 0.01 --decay 0.95
    
    #################MACHINE CONFIG#################
    # # NUMA Node        : 4
    # # Thread/NUMA Node : 20
    ################################################
    
    #################GIBBS SAMPLING#################
    # input_folder       : data/
    # output_folder      : data/
    # n_learning_epoch   : 100
    # n_samples/l. epoch : 10
    # n_inference_epoch  : 100
    # stepsize           : 0.01
    # decay              : 0.95
    ################################################
    # IGNORE -s (n_samples/l. epoch). ALWAYS -s 1. #
    ################################################
    LOADED VARIABLES: #854944
              QUERY : #813553
              EVID  : #41391
    LOADED FACTORS: #8457638
    LOADED WEIGHTS: #570540
    LOADED EDGES: #8457638
    FACTOR GRAPH: Safety Checking Passed...
    Start copying factor graph (immutable part) to node 0
    Start copying factor graph (mutable part) to node 0
    Start copying factor graph (immutable part) to node 1
    Start copying factor graph (mutable part) to node 1
    Start copying factor graph (immutable part) to node 2
    Start copying factor graph (mutable part) to node 2
    Start copying factor graph (immutable part) to node 3
    Start copying factor graph (mutable part) to node 3
    LEARNING EPOCH 0~4....0.19 sec.,1.8e+07 vars/sec.,stepsize=0.01,lmax=2.4e+02,l2=1.2e+03
    LEARNING EPOCH 4~8....0.16 sec.,2.2e+07 vars/sec.,stepsize=0.0095,lmax=1.1e+02,l2=8.3e+02
    LEARNING EPOCH 8~12....0.16 sec.,2.2e+07 vars/sec.,stepsize=0.009,lmax=88,l2=7.1e+02
    LEARNING EPOCH 12~16....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0086,lmax=90,l2=6.8e+02
    LEARNING EPOCH 16~20....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0081,lmax=85,l2=6.6e+02
    LEARNING EPOCH 20~24....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0077,lmax=86,l2=6.5e+02
    LEARNING EPOCH 24~28....0.19 sec.,1.8e+07 vars/sec.,stepsize=0.0074,lmax=1.1e+02,l2=6.5e+02
    LEARNING EPOCH 28~32....0.19 sec.,1.8e+07 vars/sec.,stepsize=0.007,lmax=1.1e+02,l2=6.2e+02
    LEARNING EPOCH 32~36....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0066,lmax=1.3e+02,l2=6.4e+02
    LEARNING EPOCH 36~40....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0063,lmax=1e+02,l2=6.2e+02
    LEARNING EPOCH 40~44....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.006,lmax=89,l2=6.1e+02
    LEARNING EPOCH 44~48....0.16 sec.,2.2e+07 vars/sec.,stepsize=0.0057,lmax=1e+02,l2=6.2e+02
    LEARNING EPOCH 48~52....0.17 sec.,2e+07 vars/sec.,stepsize=0.0054,lmax=99,l2=6.1e+02
    LEARNING EPOCH 52~56....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0051,lmax=91,l2=6.1e+02
    LEARNING EPOCH 56~60....0.16 sec.,2.1e+07 vars/sec.,stepsize=0.0049,lmax=1.9e+02,l2=6.4e+02
    LEARNING EPOCH 60~64....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0046,lmax=1.5e+02,l2=6.3e+02
    LEARNING EPOCH 64~68....0.18 sec.,2e+07 vars/sec.,stepsize=0.0044,lmax=1.6e+02,l2=6.3e+02
    LEARNING EPOCH 68~72....0.19 sec.,1.8e+07 vars/sec.,stepsize=0.0042,lmax=1e+02,l2=6.1e+02
    LEARNING EPOCH 72~76....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.004,lmax=89,l2=6.2e+02
    LEARNING EPOCH 76~80....0.19 sec.,1.8e+07 vars/sec.,stepsize=0.0038,lmax=96,l2=6e+02
    LEARNING EPOCH 80~84....0.17 sec.,2.1e+07 vars/sec.,stepsize=0.0036,lmax=1.2e+02,l2=6e+02
    LEARNING EPOCH 84~88....0.17 sec.,2e+07 vars/sec.,stepsize=0.0034,lmax=1.9e+02,l2=6.9e+02
    LEARNING EPOCH 88~92....0.19 sec.,1.8e+07 vars/sec.,stepsize=0.0032,lmax=1.9e+02,l2=6.8e+02
    LEARNING EPOCH 92~96....0.18 sec.,1.9e+07 vars/sec.,stepsize=0.0031,lmax=1.2e+02,l2=5.9e+02
    LEARNING EPOCH 96~100....0.18 sec.,2e+07 vars/sec.,stepsize=0.0029,lmax=1.5e+02,l2=6.4e+02
    TOTAL LEARNING TIME: 4.4 sec.
    LEARNING SNIPPETS (QUERY WEIGHTS):
       0 -2.1
       1 -0.00057
       2 -5.1e-05
       3 -0.00037
       4 0.011
       5 0.0074
       6 -0.0014
       7 -0.016
       8 -0.0054
       9 0.02
       ...
    DUMPING... PROTOCOL: data//inference_result.out.weights
    DUMPING... TEXT    : data//inference_result.out.weights.text
    INFERENCE EPOCH 0~4....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 4~8....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 8~12....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 12~16....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 16~20....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 20~24....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 24~28....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 28~32....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 32~36....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 36~40....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 40~44....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 44~48....0.13 sec.,2.7e+07 vars/sec
    INFERENCE EPOCH 48~52....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 52~56....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 56~60....0.13 sec.,2.7e+07 vars/sec
    INFERENCE EPOCH 60~64....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 64~68....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 68~72....0.12 sec.,2.8e+07 vars/sec
    INFERENCE EPOCH 72~76....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 76~80....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 80~84....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 84~88....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 88~92....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 92~96....0.12 sec.,2.9e+07 vars/sec
    INFERENCE EPOCH 96~100....0.12 sec.,2.9e+07 vars/sec
    TOTAL INFERENCE TIME: 3.1 sec.
    INFERENCE SNIPPETS (QUERY VARIABLES):
       0 0.55  @  1e+02
       1 0.21  @  1e+02
       2 0.27  @  1e+02
       3 0  @  1e+02
       4 0.02  @  1e+02
       5 0.43  @  1e+02
       7 0  @  1e+02
       8 0.09  @  1e+02
       9 0  @  1e+02
       10 0.06  @  1e+02
       ...
    DUMPING... PROTOCOL: data//inference_result.out
    DUMPING... TEXT    : data//inference_result.out.text
    INFERENCE CALIBRATION (QUERY BINS):
    PROB BIN 0.0~0.1  -->  # 365561
    PROB BIN 0.1~0.2  -->  # 105145
    PROB BIN 0.2~0.3  -->  # 67195
    PROB BIN 0.3~0.4  -->  # 54179
    PROB BIN 0.4~0.5  -->  # 46440
    PROB BIN 0.5~0.6  -->  # 39686
    PROB BIN 0.6~0.7  -->  # 33343
    PROB BIN 0.7~0.8  -->  # 29016
    PROB BIN 0.8~0.9  -->  # 26368
    PROB BIN 0.9~0.10  -->  # 46620
