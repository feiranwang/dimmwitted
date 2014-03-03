

all:
	g++ -O3 -I./lib/tclap/include/            	\
        src/main.cpp							\
    -o dw


gibbs:
	./dw -i data/ -o data/ -n 10