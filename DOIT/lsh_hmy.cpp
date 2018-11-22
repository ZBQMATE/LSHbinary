#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>

#include <bitset>
#include <algorithm>
#include <cstdlib>
#include <ctime>


#include <vector>
#include "vector.hpp"
#include "vector.cpp"
#include "table.hpp"
#include "table.cpp"



using namespace std;

int main() {
	
	ifstream indata("100ksift_lda_64_binary.txt");
	ifstream inquery("100ksift_lda_64_binary_query.txt");
	
	ofstream dist_result("lsh1000dist.txt");
	ofstream time_report("time_report.txt");
	
	
	/**
	* SET PARAMETERS HERE
	*/
	short unsigned int table_sample = 5;
	short unsigned int table_partition = 1;
	///////////////////////
	
	
	lsh::table lsh_table({.dimensions = 64, .samples = table_sample, .partitions = table_partition});
	
	/**
	* MAKE DATA
	*/
	
	cout << "make data ..." << endl;
	
	string tmp_line;
	string sub_line;
	for (int i = 0; i < 100000; i++) {
		getline(indata, tmp_line);
		
		vector<bool> tmp_std_vec;
		
		for (int j = 0; j < 64; j++){
			sub_line = tmp_line.substr(j, j);
			int rec_bit = atoi(sub_line.c_str());
			
			tmp_std_vec.push_back(rec_bit);
		}
		
		lsh::vector tmp_lsh_vec(tmp_std_vec);
		
		lsh_table.insert(tmp_lsh_vec);
		
	}
	
	
	/**
	* MAKE QUERIES
	* TIME HERE
	*/
	
	clock_t ttt;
	double samsung;
	ttt = clock();
	
	for (int i = 0; i < 1000; i++) {
		cout << "make query ...  ";
		cout << i << endl;
		
		getline(inquery, tmp_line);
		vector<bool> tmp_std_vec_b;
		for (int j = 0; j < 64; j++) {
			sub_line = tmp_line.substr(j, j);
			int rcd_bit = atoi(sub_line.c_str());
			tmp_std_vec_b.push_back(rcd_bit);
		}
		
		lsh::vector tmp_qry_lsh_vec(tmp_std_vec_b);
		
		// QUERY
		lsh::vector rst_vec = lsh_table.query(tmp_qry_lsh_vec);
		
		// calculate the distance and write to output file
		int tmp_dist = lsh::vector::distance(rst_vec, tmp_qry_lsh_vec);
		
		dist_result << tmp_dist << endl;
		
	}
	
	ttt = clock() - ttt;
	samsung = ttt / (double)CLOCKS_PER_SEC;
	time_report << samsung << endl;
	
	return 0;
}