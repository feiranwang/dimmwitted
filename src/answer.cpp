
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <vector>

int main(int argc, char ** argv){

	std::string filename_mat = std::string(argv[1]);
	std::string filename_query = std::string(argv[2]);
	std::string filename_key = std::string(argv[3]);
	int n_epoch = atoi(argv[4]);
	int n_proj = atoi(argv[5]);
	int n_query = atoi(argv[6]);
	int n_condition = atoi(argv[7]);

	// FIRST LOAD DOMAIN
	std::cout << "LOADING DOMAINS...";
	std::ifstream fin((filename_mat + "/QA.domain").c_str());
	int nvar;
	fin >> nvar;
	int * domains = new int[nvar];
	int * assignments = new int[nvar];
	int ct = 0;
	int domain;
	while(fin >> domain){
		domains[ct++] = domain;
	}
	std::cout << "/DONE." << std::endl;

	std::ifstream fin2((filename_query).c_str());

	bool valid = false;
	int var;
	std::vector<std::vector<int> > projections;
	std::vector<std::vector<int> > queries;
	std::vector<std::vector<int> > conditions;
	while(true){
		ct = 0;
		std::vector<int> projection;
		std::vector<int> query;
		std::vector<int> condition;
		while(ct < n_proj + n_query + n_condition){
			valid = (fin2 >> var);
			if(!valid){
				break;
			}
			if(ct < n_proj){
				projection.push_back(var);
			}else if(ct < n_proj + n_query){
				query.push_back(var);
			}else if(ct < n_proj + n_query + n_condition){
				condition.push_back(var);
			}
			ct ++;
		}
		if(!valid){
			break;
		}
		projections.push_back(projection);
		queries.push_back(query);
		conditions.push_back(condition);
	}
	std::cout << "Loaded # Projections = " << projections.size() << std::endl;
	std::cout << "Loaded # Queries     = " << queries.size() << std::endl;
	std::cout << "Loaded # Conditions  = " << conditions.size() << std::endl;
	fin2.close();

	std::ifstream fin3(filename_key.c_str());
	ct = 0;
	std::vector<int> projection_ans;
	std::vector<int> query_ans;
	std::vector<int> condition_ans;
	while(ct < n_proj + n_query + n_condition){
		valid = (fin3 >> var);
		if(!valid){
			break;
		}
		if(ct < n_proj){
			projection_ans.push_back(var);
		}else if(ct < n_proj + n_query){
			query_ans.push_back(var);
		}else if(ct < n_proj + n_query + n_condition){
			condition_ans.push_back(var);
		}
		ct ++;
	}
	std::cout << "Loaded # Projections ANS = " << projection_ans.size() << std::endl;
	std::cout << "Loaded # Queries ANS     = " << query_ans.size() << std::endl;
	std::cout << "Loaded # Conditions ANS  = " << condition_ans.size() << std::endl;

	int * tally_querytrue = new int[projections.size()];
	int * tally_conditiontrue = new int[projections.size()];
	for(int i=0;i<projections.size();i++){
		tally_querytrue[i] = 0;
		tally_conditiontrue[i] = 0;
	}

	// start tally
	for(int i_epoch=0;i_epoch<n_epoch;i_epoch++){
		std::cout << "START EPOCH " << i_epoch << std::endl;

		std::ifstream fin4((filename_mat + "/QA.epoch." + std::to_string(i_epoch)).c_str());
		ct = 0;
		while(fin4 >> var){
			assignments[ct++] = var;
		}
		std::cout << "  | LOADED. START TALLY." << std::endl;

		for(int i=0;i<projections.size();i++){
			const std::vector<int> & iproj = projections[i];
			const std::vector<int> & iquery = queries[i];
			const std::vector<int> & icondition = conditions[i];

			if(iproj.size() == 0){
				bool is_query_true=true;
				bool is_condition_true=true;

				for(int j=0;j<iquery.size();j++){
					if(assignments[iquery[j]] != query_ans[j]){
						is_query_true = false;
					}
				}

				for(int j=0;j<icondition.size();j++){
					if(assignments[icondition[j]] != condition_ans[j]){
						is_condition_true = false;
					}
				}

				if(is_query_true){
					tally_querytrue[i] ++;
				}
				if(is_condition_true){
					tally_conditiontrue[i] ++;
				}
			}else{
				assert(false);
			}

		}

		std::cout << "  | DONE." << std::endl;

	}

	std::ofstream qars((filename_mat + "/QA.rs").c_str());
	std::cout << "DUMPING RESULT TO " << (filename_mat + "/QA.rs") << std::endl;
	for(int i=0;i<projections.size();i++){
		const std::vector<int> & iproj = projections[i];
		const std::vector<int> & iquery = queries[i];
		const std::vector<int> & icondition = conditions[i];

		for(int j=0;j<iproj.size();j++){
			qars << iproj[j] << '\t';
		}

		for(int j=0;j<iquery.size();j++){
			qars << iquery[j] << '\t';
		}

		for(int j=0;j<icondition.size();j++){
			qars << icondition[j] << '\t';
		}

		qars << (1.0*tally_querytrue[i])/tally_conditiontrue[i] << std::endl;

	}
	qars.close();
	std::cout << "  | DONE." << std::endl;
	return 0;
}












