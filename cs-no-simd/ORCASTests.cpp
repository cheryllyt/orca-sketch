#include <iostream>
#include <fstream>
#include <chrono>
#include <unordered_map>

#include "ORCAS.hpp"
#include "ORCASTests.hpp"

#define FT_SIZE 13 // TODO: move to 'Defs' file

using namespace std;

// test referenced from: https://github.com/SALSA-ICDE2021/SALSA
void test_orcas_error_on_arrival(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data)
{
    ORCASketch orca_sketch;
    orca_sketch.initialize(sketch_size, number_of_arrays, number_of_array_counters, seed);

    unordered_map<uint64_t, uint64_t> true_sizes;

    int64_t stop_loop = N * FT_SIZE;

    long double L1e = 0, L2e = 0, L_max = 0;

	BOBHash ft_to_bobkey_1(seed);
	BOBHash ft_to_bobkey_2(seed + 17);

    for (int64_t i = 0; i < stop_loop; i += FT_SIZE)
    {
        uint64_t ft_key = (((uint64_t)ft_to_bobkey_1.run(data + i, FT_SIZE)) << 32) + (uint64_t)ft_to_bobkey_2.run(data + i, FT_SIZE);

        if (true_sizes.find(ft_key) == true_sizes.end())
		{
			true_sizes[ft_key] = 0;
		}

        long double point_error = (int64_t)orca_sketch.query(data + i) - (int64_t)true_sizes[ft_key];

        L1e += abs(point_error);
		L2e += (point_error * point_error);
		L_max = (L_max < abs(point_error)) ? abs(point_error) : L_max;

		true_sizes[ft_key] += 1;
		orca_sketch.increment(data + i);
    }

    L1e /= N;
	L2e /= N;
	L2e = sqrt(L2e);

    ofstream results_file;
    string folder_name = "ORCAS-test-results/";
    string seed_str = "seed_" + to_string(seed);
    string file_name = folder_name + seed_str + "_test_orcas_error_on_arrival.txt";
    results_file.open(file_name, ofstream::out | ofstream::app);
	results_file << "N\t" << N
                 << "\tSketch_Size\t" << sketch_size
                 << "\tNo_Arrays\t" << number_of_arrays
                 << "\tNo_A_Counters\t" << number_of_array_counters
                 << "\tL1 Error\t" << L1e
                 << "\tL2 Error\t" << L2e
                 << "\tL(inf) Error\t" << L_max
                 << endl;
}

// test referenced from: https://github.com/SALSA-ICDE2021/SALSA
void test_orcas_speed(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data)
{
    ORCASketch orca_sketch;
    orca_sketch.initialize(sketch_size, number_of_arrays, number_of_array_counters, seed);

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
                 << "\tNo_Arrays\t" << number_of_arrays
                 << "\tNo_A_Counters\t" << number_of_array_counters
                 << "\tTime\t" << time
                 << endl;
}
