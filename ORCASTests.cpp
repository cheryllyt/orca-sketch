// #include <stdint.h>
// #include <stdlib.h>
#include <iostream>
#include <fstream>
#include <chrono>
// #include <math.h>
// #include <time.h>
// #include <unordered_map>

#include "ORCAS.hpp"
#include "ORCASTests.hpp"

#define FT_SIZE 13 // TODO: move to 'Defs' file

using namespace std;

void test_orcas_error_on_arrival()
{

}

void test_orcas_speed(int N, int sketch_size, int number_of_buckets, int number_of_bucket_counters, int seed, const char* data)
{
    ORCASketch orca_sketch;
    orca_sketch.initialize(sketch_size, number_of_buckets, number_of_bucket_counters, seed);

    int64_t stop_loop = N * FT_SIZE;

    auto start = chrono::steady_clock::now();
    for (int64_t i = 0; i < stop_loop; i += FT_SIZE)
    {
        orca_sketch.increment(data + i);
    }
    auto end = chrono::steady_clock::now();

    auto time = chrono::duration_cast<chrono::microseconds>(end - start).count();

    ofstream results_file;
    string folder_name = "ORCAS-test-results/";
    string seed_str = "seed_" + to_string(seed);
    string file_name = folder_name + seed_str + "_test_orcas_speed.txt";
    results_file.open(file_name, ofstream::out | ofstream::app);
	results_file << "N\t" << N
                 << "\tSketch_Size\t" << sketch_size
                 << "\tNo_Buckets\t" << number_of_buckets
                 << "\tNo_B_Counters\t" << number_of_bucket_counters
                 << "\tTime\t" << time
                 << endl;
}
