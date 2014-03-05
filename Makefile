

all:
	/progs/gcc-4.7/bin/g++-4.7 -Ofast -std=c++0x						\
	        -I./lib/tclap/include/            							\
	        -I./lib/protobuf/include/									\
	        -L./lib/protobuf/lib/ 										\
	        -I./src														\
	        -lnuma -lrt	-lprotobuf										\
	    src/dstruct/factor_graph.pb.cc src/dstruct/factor_graph.cpp 	\
	    src/app/gibbs/gibbs_sampling.cpp 								\
	    src/main.cpp													\
    -o dw


gibbs:
	./dw -i data/ -o data/ -n 10

test:
	./dw -i ./test/factor_graph/lr_inf/ -o ./test/factor_graph/lr_inf/ -n 10

test_learn:
	./dw -i ./test/factor_graph/lr_learn/ -o ./test/factor_graph/lr_learn/ -n 10


gibbs_pb:
	./lib/protobuf/bin/protoc -I=./src/dstruct/ --cpp_out=./src/dstruct/ ./src/dstruct/factor_graph.proto

gibbs_pb_py:
	./lib/protobuf/bin/protoc -I=./src/dstruct/ --python_out=./test/factor_graph ./src/dstruct/factor_graph.proto