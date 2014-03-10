DimmWitted
==========

# How fast is DimmWitted?

  - On Amazon EC2's FREE MACHINE (512M memory, 1 core). We can sample 3.6M varialbes/seconds.
  - On a 2-node Amazon EC2 machine, sampling 7 billion random variables, each of which has 10 features, takes 3 minutes. This means we can run inference for all living human beings on this planet with $15 (100 samples!)
  - On Macbook, DimmWitted runs 10x faster than DeepDive's default sampler.

# Pre-built Binary

We include a pre-built binary in the release folder for both Linux and Mac... 
Because we reply on lnuma, this is not always possible... But 
we successfully deployed this binary to the following configurations. Good luck!

  - Macbook or Apple Server
    - **(Mac OSX > 10.7 should work)**
    - Macbook Pro: Darwin MACHINENAME 13.0.0 Darwin Kernel Version 13.0.0: Thu Sep 19 22:22:27 PDT 2013; root:xnu-2422.1.72~6/RELEASE_X86_64 x86_64
    - Apple Server: Darwin MACHINENAME 10.8.0 Darwin Kernel Version 10.8.0: Tue Jun  7 16:33:36 PDT 2011; root:xnu-1504.15.3~1/RELEASE_I386 i386
  - Linux Servers:
    - **(Newer than 2006-released Linux kernel 2.6.18 should work)** 
    - Local Machine: Linux MACHINENAME 2.6.32-358.23.2.el6.x86_64 #1 SMP Sat Sep 14 05:32:37 EDT 2013 x86_64 x86_64 x86_64 GNU/Linux
    - Local Machine: Linux MACHINENAME 2.6.32-431.3.1.el6.x86_64 #1 SMP Fri Dec 13 06:58:20 EST 2013 x86_64 x86_64 x86_64 GNU/Linux
    - Local Machine: Linux MACHINENAME 2.6.32-431.3.1.el6.x86_64 #1 SMP Fri Dec 13 06:58:20 EST 2013 x86_64 x86_64 x86_64 GNU/Linux
    - Local Machine: Linux MACHINENAME 2.6.32-279.el6.x86_64 #1 SMP Fri Jun 22 12:19:21 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux
    - Local Machine: Linux MACHINENAME 2.6.32-358.el6.x86_64 #1 SMP Fri Feb 22 00:31:26 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux
    - Local Machine: Linux MACHINENAME 2.6.18-308.el5 #1 SMP Tue Feb 21 20:06:06 EST 2012 x86_64 x86_64 x86_64 GNU/Linux
    - Local Machine: Linux MACHINENAME 2.6.18-274.12.1.el5 #1 SMP Tue Nov 8 21:37:35 EST 2011 x86_64 x86_64 x86_64 GNU/Linux
  - EC2 Machines
    - **(The cheapest ones with Ubuntu12.04 worked!)**
    - EC2 Free Machine: Linux MACHINENAME 3.2.0-58-virtual #88-Ubuntu SMP Tue Dec 3 17:58:13 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux
  
We haven't found machines that cannot work with this binary yet.

If you are lucky, the follow two commands will tell you whether it works or not

    tar xf dw.tar.bz2
    sh test.sh

For Mac, the filename is

    dw_mac.tar.bz2
    
instead. We assume at least MAC OSX 10.7.

# Installation

Sorry if you need to read this section to build DimmWitted by yourself. But fortunately, it is not that hard.

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
