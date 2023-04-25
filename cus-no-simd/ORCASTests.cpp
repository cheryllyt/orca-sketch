#include <iostream>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <vector>

#include "ORCAS.hpp"
#include "ORCASTests.hpp"

#include "Defs.hpp"

using namespace std;

// test referenced from: https://github.com/SALSA-ICDE2021/SALSA [1]
template<int number_of_options_ind>
void test_orcas_error_on_arrival(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data)
{
    ORCASketch<number_of_options_ind> orca_sketch;
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

// test referenced from: https://github.com/SALSA-ICDE2021/SALSA [1]
template<int number_of_options_ind>
void test_orcas_speed(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data)
{
    ORCASketch<number_of_options_ind> orca_sketch;
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

// test referenced from: https://github.com/SALSA-ICDE2021/SALSA [1]
template<int number_of_options_ind>
void test_orcas_final_error(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data)
{
    ORCASketch<number_of_options_ind> orca_sketch;
    orca_sketch.initialize(sketch_size, number_of_arrays, number_of_array_counters, seed);

    unordered_map<uint64_t, uint64_t> true_sizes;
	unordered_map<uint64_t, uint64_t> ft_key_i_values;

	unordered_map<double, vector<uint64_t>> threshold_to_hh_ft_keys;
	vector<double> thresholds = { 0.0001, 0.000178, 0.000316, 0.000562, 0.001, 0.00178, 0.00316, 0.00562, 0.01 };
	for (int i = 0; i < thresholds.size(); ++i)
	{
		threshold_to_hh_ft_keys[thresholds[i]] = vector<uint64_t>();
	}

	int64_t stop_loop = N * FT_SIZE;

    BOBHash ft_to_bobkey_1(seed);
	BOBHash ft_to_bobkey_2(seed + 17);

	for (int64_t i = 0; i < stop_loop; i += 13)
	{
		uint64_t ft_key = (((uint64_t)ft_to_bobkey_1.run(data + i, FT_SIZE)) << 32) + (uint64_t)ft_to_bobkey_2.run(data + i, FT_SIZE);

		if (true_sizes.find(ft_key) == true_sizes.end())
		{
			true_sizes[ft_key] = 0;
		}
		ft_key_i_values[ft_key] = i;
		true_sizes[ft_key] += 1;
		orca_sketch.increment(data + i);
	}

    for (auto it = true_sizes.begin(); it != true_sizes.end(); it++)
	{
		for (int i = 0; i < thresholds.size(); ++i)
		{
			if (it->second >= thresholds[i] * N)
			{
				threshold_to_hh_ft_keys[thresholds[i]].push_back(it->first);
			}
			else
			{
				break;
			}
		}
	}

    vector<long double> HHre;
	for (int i = 0; i < thresholds.size(); ++i)
	{
		long double current_hh_rl = 0;
		for (int j = 0; j < threshold_to_hh_ft_keys[thresholds[i]].size(); ++j)
		{
			uint64_t current_ftkey = threshold_to_hh_ft_keys[thresholds[i]][j];
			long double current_query = orca_sketch.query(data + ft_key_i_values[current_ftkey]);
			current_hh_rl += (long double)abs(current_query - true_sizes[current_ftkey]) / (long double)true_sizes[current_ftkey];
		}
		current_hh_rl /= threshold_to_hh_ft_keys[thresholds[i]].size();
		HHre.push_back(current_hh_rl);
	}

    ofstream results_file;
    string folder_name = "ORCAS-test-results/";
    string seed_str = "seed_" + to_string(seed);
    string file_name = folder_name + seed_str + "_test_orcas_final_error.txt";
    results_file.open(file_name, ofstream::out | ofstream::app);
	results_file << "N\t" << N
                 << "\tSketch_Size\t" << sketch_size
                 << "\tNo_Arrays\t" << number_of_arrays
                 << "\tNo_A_Counters\t" << number_of_array_counters;
    for (int i = 0; i < HHre.size(); ++i)
	{
		results_file << "\tThreshold\t" << thresholds[i] << "\tRelError\t" << HHre[i];
	}
	results_file << endl;
}

// template explicit instantiations
template void test_orcas_error_on_arrival<OPTION_64C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_64C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_64C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_32C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_32C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_32C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_8C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_8C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_8C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_4C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_4C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_4C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_16C2>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_16C2>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_16C2>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_16C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_16C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_16C3>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_16C4>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_16C4>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_16C4>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_16C5>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_16C5>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_16C5>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_16C6>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_16C6>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_16C6>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template void test_orcas_error_on_arrival<OPTION_16C7>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_speed<OPTION_16C7>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
template void test_orcas_final_error<OPTION_16C7>(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);
