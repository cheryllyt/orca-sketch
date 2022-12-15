#pragma once

#ifndef ORCA_SKETCH
#define ORCA_SKETCH

#include <Python/Python.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <charconv>

#include "salsa-src/BobHash.hpp"

class ORCASketch {

	int sketch_size;
	int number_of_buckets;
    int number_of_bucket_counters;

    int bucket_size;
    int bucket_mask;

    int number_of_options; 
    int option_mask;
    int **bucket_counter_lookup_table;

    uint32_t *orca_sketch;
    BOBHash bobhash;

    int get_number_of_lookup_table_options();
    int **create_bucket_counter_lookup_table();

public:

	ORCASketch();
    ~ORCASketch();

	void initialize(int sketch_size, int bucket_size, int number_of_bucket_counters, int seed);
	void increment(const char * str);
	uint32_t query(const char * str);

};

#endif // !ORCA_SKETCH
