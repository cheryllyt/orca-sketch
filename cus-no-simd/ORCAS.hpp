#pragma once

#ifndef ORCA_SKETCH
#define ORCA_SKETCH

#include <Python/Python.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <charconv>

#include "salsa-src/BobHash.hpp"

class ORCASketch {

	int sketch_size;
	int number_of_arrays;
    int number_of_array_counters;

    int option_row_size;
    int option_row_size_bits; // number of bits required to store option row size

    int number_of_bits_array; // number of bits required to store array number

    int array_size;
    int array_mask;

    int number_of_bits_array_size; // number of bits required to store array size

    int number_of_options; 
    int number_of_options_ind;

    uint8_t *array_counter_ind_lookup_table;

    uint32_t *orca_sketch;
    BOBHash bobhash;

    void set_option_row_size(bool n_array_counter_is_pow_2);
    void set_number_of_lookup_table_options();
    void create_lookup_table(bool n_array_counter_is_pow_2);

public:

	ORCASketch();
    ~ORCASketch();

	void initialize(int sketch_size, int array_size, int number_of_array_counters, int seed);
	void increment(const char * str);
	uint32_t query(const char * str);

};

#endif // !ORCA_SKETCH