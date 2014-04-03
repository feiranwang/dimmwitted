
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <machine/endian.h>
#include <stdint.h>

#include "dstruct/factor_graph/factor_graph.h"

using namespace std;

#ifndef BINARY_PARSER_H
#define BINARY_PARSER_H


// Read meta data file, return Meta struct 
Meta read_meta(string meta_file)
{
	ifstream file;
	file.open(meta_file.c_str());
	string buf;
	Meta meta;
	getline(file, buf, ',');
	meta.num_weights = atoll(buf.c_str());
	getline(file, buf, ',');
	meta.num_variables = atoll(buf.c_str());
	getline(file, buf, ',');
	meta.num_factors = atoll(buf.c_str());
	getline(file, buf, ',');
	meta.num_edges = atoll(buf.c_str());
	getline(file, meta.weights_file, ',');
	getline(file, meta.variables_file, ',');
	getline(file, meta.factors_file, ',');
	getline(file, meta.edges_file, ',');
	file.close();
	return meta;
}

// Read weights and load into factor graph
long long read_weights(string filename, dd::FactorGraph &fg)
{
	ifstream file;
    file.open(filename.c_str(), ios::in | ios::binary);
    long long count = 0;
    long long id;
    bool isfixed;
    char padding;
    double initial_value;
    while (file.good()) {
    	
    	// read fields
        file.read((char *)&id, 8);
        file.read((char *)&padding, 1);
        if (!file.read((char *)&initial_value, 8)) break;
        
        // convert endian
        id = be64toh(id);
        isfixed = padding;
        long long tmp = be64toh(*(uint64_t *)&initial_value);
        initial_value = *(double *)&tmp;

        // load into factor graph
        fg.weights[fg.c_nweight] = dd::Weight(id, initial_value, isfixed);
		fg.c_nweight++;

        //printf("id=%lli isfixed=%d initial=%f\n", id, isfixed, initial_value);
    }
    file.close();
}


// Read variables
long long read_variables(string filename)
{

}


#endif