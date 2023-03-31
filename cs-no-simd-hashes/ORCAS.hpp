#pragma once

#ifndef ORCA_SKETCH
#define ORCA_SKETCH

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

    int bucket_size;
    int bucket_number_mask;
    int bucket_counter_mask;

    int number_of_bits_bucket_size; // number of bits required to store bucket size

    int number_of_hash_functions;

    int32_t *orca_sketch;
    BOBHash *bobhash;

public:

	ORCASketch();
    ~ORCASketch();

	void initialize(int sketch_size, int bucket_size, int number_of_bucket_counters, int seed);
	void increment(const char * str);
	uint32_t query(const char * str);

};

#endif // !ORCA_SKETCH
