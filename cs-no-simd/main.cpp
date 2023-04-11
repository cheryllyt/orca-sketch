#include <iostream>
#include <fstream>

#include "salsa-src/genzipf.h"

#include "ORCAS.hpp"
#include "ORCASTests.hpp"

#include "Defs.hpp"

using namespace std;

/*
    no SIMD version
    ===============

    data structure:
    SUMMARY - one row of counters, split into arrays
    let W = total number of counters (in the row)
    let B = size of a array (in number of counters)
    therefore, number of arrays = W / B
    let A = number of counters an item should be mapped into within a array
    therefore, number of options for which A counters within a array an item should be mapped into
             = number of rows in lookup table (of counter combinations)
             = B choose A (combination) 

    algorithm:
    1. Use BobHash (the hash function) on an item -> returns 32 bits
    2. Split into 22 bits and 10 (least significant) bits
    3. Use the 10 bits to map into a array
    4. Use the 22 bits to map into a row in the lookup table

    command to compile on MacBook:
    ------------------------------
    g++ -std=c++17 -O3 main.cpp ORCAS.cpp ORCASTests.cpp salsa-src/BobHash.cpp -framework Python -DDEBUG -DCOUNTERONE
    e.g. ./a.out 10 42 1 32 4; ./a.out 10000000 42 -1 1024 128
*/

#ifdef DEBUG
template<int number_of_options_ind>
void run_debug_code(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data)
{
    ORCASketch<number_of_options_ind> orcasketch;
    orcasketch.initialize(sketch_size, number_of_arrays, number_of_array_counters, seed);

    int64_t stop_loop = N * FT_SIZE;
    for (int64_t i = 0; i < stop_loop; i += FT_SIZE)
    {
        orcasketch.increment(data + i);
        orcasketch.query(data + i);
    }
}
#endif

int main(int argc, char* argv[])
{
    if (argc < 6) {
        cout << "Usage Error:\n";
        cout << "argv[1]: int N\n";
        cout << "argv[2]: int seed\n";
        cout << "argv[3]: float alpha\n";
        cout << "argv[4]: int sketch_size\n";
        cout << "argv[5]: int number_of_arrays\n";
        system("pause");
		return 0;
    }

    // for data generation
    int N = atoi(argv[1]); // largest: 99000000
    int seed = atoi(argv[2]); // e.g. 42
    float alpha = atof(argv[3]); // e.g. 0.6, 0.8, 1, 1.2, 1.5

    // for creating ORCA sketch
    int sketch_size = atoi(argv[4]); // e.g. 32
    int number_of_arrays = atoi(argv[5]); // e.g. 4

    // for choosing correct <number_of_options_ind>
    int array_size = (int)(sketch_size / number_of_arrays);

    // hardcoded median computations for efficiency
    #ifdef COUNTERONE
    int number_of_array_counters = 1;
    #elif defined COUNTERFIVE
    int number_of_array_counters = 5;
    #else // default
    int number_of_array_counters = 3;
    #endif

    char path[] = "./zipf";
    char* data = new char[FT_SIZE * N]();

    // generate zipf file
    if (alpha > 0)
    { 
        genzipf(path, seed, alpha, 1 << 24, N);
        cout << "\nzipf generation complete!\n";
    }
    
    // read zipf contents into data[]
    int64_t remaining = N;
    int read_so_far = 0;

    ifstream f(path, ios::binary);

    while (remaining > 0)
    {
        int64_t to_read = remaining > 100000 ? 100000 : remaining;
        f.read(data + read_so_far * FT_SIZE, to_read * FT_SIZE);
        remaining -= to_read;
        read_so_far += to_read;
    }
    
    #ifdef DEBUG // ORCA sketch driver code
    if (array_size == 16)
    {
        if (number_of_array_counters == 1)
        {
            run_debug_code<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 3)
        {
            run_debug_code<OPTION_16C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 5)
        {
            run_debug_code<OPTION_16C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 64)
    {
        if (number_of_array_counters == 3)
        {
            run_debug_code<OPTION_64C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 32)
    {
        if (number_of_array_counters == 3)
        {
            run_debug_code<OPTION_32C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 8)
    {
        if (number_of_array_counters == 3)
        {
            run_debug_code<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 4)
    {
        if (number_of_array_counters == 3)
        {
            run_debug_code<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    #endif

    /*
        memory = sketch_size x size of counter (4 bytes - size of integer)
        influences -> number_of_arrays & number_of_array_counters (hence also number_of_options)
        (1) x-axis = memory; y-axis = error (L2)
        (2) x-axis = memory; y-axis = speed/throughput (N / time)
    */

    #ifndef DEBUG // ORCA sketch tests
    if (array_size == 16)
    {
        if (number_of_array_counters == 1)
        {
            test_orcas_error_on_arrival<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 3)
        {
            test_orcas_error_on_arrival<OPTION_16C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_16C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_16C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 5)
        {
            test_orcas_error_on_arrival<OPTION_16C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_16C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_16C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 64)
    {
        if (number_of_array_counters == 3)
        {
            test_orcas_error_on_arrival<OPTION_64C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_64C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_64C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 32)
    {
        if (number_of_array_counters == 3)
        {
            test_orcas_error_on_arrival<OPTION_32C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_32C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_32C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 8)
    {
        if (number_of_array_counters == 3)
        {
            test_orcas_error_on_arrival<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    else if (array_size == 4)
    {
        if (number_of_array_counters == 3)
        {
            test_orcas_error_on_arrival<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_16C1_4C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }

    cout << "\nTests complete!\n";
    #endif

    return 0;
}
