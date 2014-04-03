#include <iostream>
#include <fstream>

//#include <stdlib.h>
#include <stdint.h>

//#include "dstruct/factor_graph/factor_graph.h"
#include "binary_parser.h"


// 64-bit big endian to little endian
# define __bswap_64(x) \
     ((((x) & 0xff00000000000000ull) >> 56)                                   \
      | (((x) & 0x00ff000000000000ull) >> 40)                                 \
      | (((x) & 0x0000ff0000000000ull) >> 24)                                 \
      | (((x) & 0x000000ff00000000ull) >> 8)                                  \
      | (((x) & 0x00000000ff000000ull) << 8)                                  \
      | (((x) & 0x0000000000ff0000ull) << 24)                                 \
      | (((x) & 0x000000000000ff00ull) << 40)                                 \
      | (((x) & 0x00000000000000ffull) << 56))

// 16-bit big endian to little endian
#define __bswap_16(x) \
     ((unsigned short int) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

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
        id = __bswap_64(id);
        isfixed = padding;
        long long tmp = __bswap_64(*(uint64_t *)&initial_value);
        initial_value = *(double *)&tmp;
        // load into factor graph
        fg.weights[fg.c_nweight] = dd::Weight(id, initial_value, isfixed);
		fg.c_nweight++;
		count++;

        //printf("id=%lli isfixed=%d initial=%f\n", id, isfixed, initial_value);
    }
    file.close();
    return count;
}


// Read variables
long long read_variables(string filename, dd::FactorGraph &fg)
{
    ifstream file;
    file.open(filename.c_str(), ios::in | ios::binary);
    long long count = 0;
    long long id;
    bool isevidence;
    char padding1;
    double initial_value;
    char type;
    short padding2;
    long long edge_count;
    long long cardinality;
    while (file.good()) {
        file.read((char *)&id, 8);
        file.read((char *)&padding1, 1);
        file.read((char *)&initial_value, 8);
        file.read((char *)&padding2, 2);
        file.read((char *)&edge_count, 8);
        if (!file.read((char *)&cardinality, 8)) break;
        id = __bswap_64(id);
        isevidence = padding1;
        type = __bswap_16(padding2);
        long long tmp = __bswap_64(*(uint64_t *)&initial_value);
        initial_value = *(double *)&tmp;
        edge_count = __bswap_64(edge_count);
        cardinality = __bswap_64(cardinality);
        count++;
        printf("id=%lli isevidence=%d initial=%f type=%c edge_count=%lli cardinality=%lli\n", id, isevidence, initial_value, type, edge_count, cardinality);

        // add to factor graph
        if (type == 'B'){
            if (isevidence) {
                fg.variables[fg.c_nvar] = dd::Variable(id, DTYPE_BOOLEAN, true, 0, 1, 
                    initial_value, initial_value, edge_count);
                fg.c_nvar++;
                fg.n_evid++;
            } else {
                fg.variables[fg.c_nvar] = dd::Variable(id, DTYPE_BOOLEAN, false, 0, 1, 
                    0, 0, edge_count);
                fg.c_nvar++;
                fg.n_query++;
            }
        } else if (type == 'M') {
            if (isevidence) {
                fg.variables[fg.c_nvar] = dd::Variable(id, DTYPE_MULTINOMIAL, true, 0, 
                    cardinality-1, initial_value, initial_value, edge_count);
                fg.c_nvar ++;
                fg.n_evid ++;
            } else {
                fg.variables[fg.c_nvar] = dd::Variable(id, DTYPE_MULTINOMIAL, false, 0, 
                    cardinality-1, 0, 0, edge_count);
                fg.c_nvar ++;
                fg.n_query ++;
            }
        } else {
            cout << "[ERROR] Only Boolean and Multinomial variables are supported now!" << endl;
            exit(1);
        }
    }
    file.close();
    cout << count << endl;
    return count;
}