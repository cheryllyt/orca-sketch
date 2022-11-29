#pragma once

#ifndef ORCA_SKETCH
#define ORCA_SKETCH

#include <stdint.h>
#include <stdlib.h>
#include <iostream>

#include "salsa-src/BobHash.hpp"

using namespace std;

class ORCASketch {

	int sketch_size;
	int number_of_buckets;
    int number_of_bucket_counters;

    int bucket_size;
    int bucket_mask;

    int number_of_options; 
    int option_mask;
    // int **bucket_counter_lookup_table;

    uint32_t *orca_sketch;
	BOBHash *bobhash;

    int factorial(int n);
    int combination();
    int **create_bucket_counter_lookup_table();

public:

	ORCASketch();
    ~ORCASketch();

    // TODO: delete
    // int **lookup_table_test(int n_options, int n_buc_count, int buc_siz);

	void initialize(int sketch_size, int bucket_size, int number_of_bucket_counters, int seed);
	void increment(const char * str);
	uint64_t query(const char * str);

};

#endif // !ORCA_SKETCH
