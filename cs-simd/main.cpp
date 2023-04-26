#include <iostream>
#include <fstream>

#include "salsa-src/genzipf.h"

#include "ORCAS.hpp"
#include "ORCASTests.hpp"

#include "Defs.hpp"

using namespace std;

/*
    command to compile on MacBook:
    ------------------------------
    g++ -std=c++17 -mavx2 -O3 main.cpp ORCAS.cpp ORCASTests.cpp salsa-src/BobHash.cpp -framework Python -DDEBUG -DCOUNTERONE
    e.g. ./a.out 10 42 1 256; ./a.out 10000000 42 -1 1024
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
    if (argc < 5) {
        cout << "Usage Error:\n";
        cout << "argv[1]: int N\n";
        cout << "argv[2]: int seed\n";
        cout << "argv[3]: float alpha\n";
        cout << "argv[4]: int sketch_size\n";
        system("pause");
		return 0;
    }

    // for data generation
    int N = atoi(argv[1]); // largest: 99000000
    int seed = atoi(argv[2]); // e.g. 42
    float alpha = atof(argv[3]); // e.g. 0.6, 0.8, 1, 1.2, 1.5

    // for creating ORCA sketch
    int sketch_size = atoi(argv[4]); // e.g. 32
    int number_of_arrays = (int)(sketch_size / ARRAY_SIZE); // array size is fixed depending on CPU

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
    if (ARRAY_SIZE == 8)
    {
        if (number_of_array_counters == 1)
        {
            run_debug_code<OPTION_8C1>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 3)
        {
            run_debug_code<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 5)
        {
            run_debug_code<OPTION_8C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
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
    if (ARRAY_SIZE == 8)
    {
        if (number_of_array_counters == 1)
        {
            test_orcas_error_on_arrival<OPTION_8C1>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_8C1>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_8C1>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 3)
        {
            test_orcas_error_on_arrival<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_8C3>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
        else if (number_of_array_counters == 5)
        {
            test_orcas_error_on_arrival<OPTION_8C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_speed<OPTION_8C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
            test_orcas_final_error<OPTION_8C5>(N, sketch_size, number_of_arrays, number_of_array_counters, seed, data);
        }
    }
    
    cout << "\nTests complete!\n";
    #endif

    return 0;
}
