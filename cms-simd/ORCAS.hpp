#pragma once

#ifndef ORCA_SKETCH
#define ORCA_SKETCH

#include <Python/Python.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <assert.h>
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

    int number_of_bits_bucket; // number of bits required to store bucket number

    int bucket_size;
    int bucket_mask;

    int number_of_options;
    int number_of_options_ind; // max index for lookup table
    
    __m256i *bucket_counter_vec_lookup_table;
    uint8_t *bucket_counter_ind_lookup_table;

    uint32_t *orca_sketch;
    BOBHash bobhash;

    void set_number_of_lookup_table_options();
    void create_lookup_tables();

public:

	ORCASketch();
    ~ORCASketch();

	void initialize(int sketch_size, int bucket_size, int number_of_bucket_counters, int seed);
	void increment(const char * str);
	uint32_t query(const char * str);

};

#endif // !ORCA_SKETCH
