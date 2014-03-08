

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
ifndef CXX
CXX = g++
endif
OPT_FLAG = -Ofast
GCC_INCLUDE = -I./lib/tclap/include/ -I./lib/protobuf/include/ -I./src
GCC_LIB = -L./lib/protobuf/lib/
CPP_FLAG = -std=c++0x -lnuma -lrt -lprotobuf
endif

ifeq ($(UNAME), Darwin)
ifndef CXX
CXX = clang++
endif
OPT_FLAG = -O3 -stdlib=libc++
GCC_INCLUDE = -I./lib/tclap/include/ -I./lib/protobuf/include/ -I./src
GCC_LIB = -L./lib/protobuf/lib/
CPP_FLAG = -std=c++0x  -lprotobuf
endif

COMPILE_CMD = $(GCC) $(OPT_FLAG) $(GCC_INCLUDE) $(GCC_LIB) $(CPP_FLAG)

dw: factor_graph.o factor_graph.pb.o gibbs_sampling.o main.o
	$(COMPILE_CMD) -o dw factor_graph.o factor_graph.pb.o gibbs_sampling.o main.o    

main.o: src/main.cpp
	$(COMPILE_CMD) -c src/main.cpp

factor_graph.o: src/dstruct/factor_graph.cpp src/io/pb_parser.h
	$(COMPILE_CMD) -c src/dstruct/factor_graph.cpp

factor_graph.pb.o: src/dstruct/factor_graph.pb.cc
	$(COMPILE_CMD) -c src/dstruct/factor_graph.pb.cc

gibbs_sampling.o: src/app/gibbs/gibbs_sampling.cpp
	$(COMPILE_CMD) -c src/app/gibbs/gibbs_sampling.cpp


dep:
ifeq ($(UNAME), Darwin)
	cd lib;\
	tar xf protobuf-2.5.0.tar.bz2;\
	cd protobuf-2.5.0;\
	./configure --prefix=`pwd`/../protobuf CC=clang CXX=clang++ CXXFLAGS='-std=c++11 -stdlib=libc++ -O3 -g' LDFLAGS='-stdlib=libc++' LIBS="-lc++ -lc++abi";\
	make -j8;\
	make install

	cd lib;\
	tar xf tclap-1.2.1.tar.gz;\
	cd tclap-1.2.1;\
	./configure --prefix=`pwd`/../tclap;\
	make;\
	make install
endif

ifeq ($(UNAME), Darwin)
	cd lib;\
	tar xf protobuf-2.5.0.tar.bz2;\
	cd protobuf-2.5.0;\
	./configure --prefix=`pwd`/../protobuf ;\
	make -j8;\
	make install

	cd lib;\
	tar xf tclap-1.2.1.tar.gz;\
	cd tclap-1.2.1;\
	./configure --prefix=`pwd`/../tclap;\
	make;\
	make install
endif


clean:
	rm -rf factor_graph.o factor_graph.pb.o gibbs_sampling.o main.o
	rm -rf dw

gibbs:
	./dw gibbs -e data/ -o data/ -i 100 -l 100 -s 10 --alpha 0.01 --decay 0.95

test:
	./dw gibbs -e ./test/factor_graph/lr_inf/ 		\
			   -o ./test/factor_graph/lr_inf/ 		\
			   -i 100 -l 100 -s 10

test_learn:
	./dw gibbs -e ./test/factor_graph/lr_learn/ 	\
			   -o ./test/factor_graph/lr_learn/ 	\
			   -i 1000 -l 1000 -s 10 --alpha 0.0001

test_learn_dep:
	./dw gibbs -e ./test/factor_graph/lr_learn_dep/ 	\
			   -o ./test/factor_graph/lr_learn_dep/ 	\
			   -i 1000 -l 100 -s 10 --alpha 0.0001

test_crf:
	./dw gibbs -e ./test/factor_graph/crf_mix/ 		\
			   -o ./test/factor_graph/crf_mix/ 		\
			   -i 1000 -l 100 -s 10 --alpha 0.0001

gibbs_pb:
	./lib/protobuf/bin/protoc -I=./src/dstruct/ --cpp_out=./src/dstruct/ ./src/dstruct/factor_graph.proto

gibbs_pb_py:
	./lib/protobuf/bin/protoc -I=./src/dstruct/ --python_out=./test/factor_graph ./src/dstruct/factor_graph.proto
