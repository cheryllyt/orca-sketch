#include <Python/Python.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "ORCAS.hpp"

#define NUMBER_OF_HASH_FUNC 4
#define BUCKET_HASH 0
#define OPTION_HASH 1
#define FT_SIZE 13

using namespace std;

ORCASketch::ORCASketch()
{
}

ORCASketch::~ORCASketch()
{
	// TODO: delete[] bucket_counter_lookup_table
    delete[] orca_sketch;
    delete[] bobhash;
}

void ORCASketch::initialize(int sketch_size, int number_of_buckets, int number_of_bucket_counters, int seed, char *py_bucket_size, char *py_number_of_bucket_counters)
{
    this->sketch_size = sketch_size;
    this->number_of_buckets = number_of_buckets;
    this->number_of_bucket_counters = number_of_bucket_counters;
    
    this->py_bucket_size = py_bucket_size;
    this->py_number_of_bucket_counters = py_number_of_bucket_counters;

    bucket_size = sketch_size / number_of_buckets;
    bucket_mask = number_of_buckets - 1;

    number_of_options = combination();
    option_mask = number_of_options - 1;
    bucket_counter_lookup_table = create_bucket_counter_lookup_table();

    cout << "\n";
    for (int i = 0; i < number_of_options; i++)
    {
        for (int j = 0; j < number_of_bucket_counters; j++)
        {
            cout << bucket_counter_lookup_table[i][j] << " ";
        }
        cout << "\n";
    }

    cout << "\nsketch_size: " << sketch_size << "\n";
    cout << "number_of_buckets: " << number_of_buckets << "\n";
    cout << "bucket_mask: " << bucket_mask << "\n";
    cout << "bucket_size: " << bucket_size << "\n";
    cout << "number_of_bucket_counters: " << number_of_bucket_counters << "\n";
    cout << "number_of_options: " << number_of_options << "\n";
    cout << "option_mask: " << option_mask << "\n";

    orca_sketch = new uint32_t[sketch_size]();

    // TODO: use 2 hash functions
    // 0: to map into bucket
    // 1: to choose which rows in lookup table

    // currently using (number_of_bucket_counters + 1) hash functions
    // 0: to map into bucket
    // 1-n: to choose which counters within a bucket to map into

    bobhash = new BOBHash[NUMBER_OF_HASH_FUNC];
    for (int i = 0; i < NUMBER_OF_HASH_FUNC; i++)
    {
        bobhash[i].initialize(seed*(7 + i) + i + 100);
    }
}

void ORCASketch::increment(const char * str)
{
    uint bucket_index = (bobhash[BUCKET_HASH].run(str, FT_SIZE)) & bucket_mask;
    cout << "\nbucket_index: " << bucket_index << "\n";

    // currently using number_of_bucket_counters hash functions
    for (int i = 1; i < (number_of_bucket_counters + 1); i++)
    {
        int bucket_counter_mask = bucket_size - 1;
        uint bucket_counter_index = (bobhash[i].run(str, FT_SIZE)) & bucket_counter_mask;
        cout << "bucket_counter_index: " << bucket_counter_index << "\n";

        // calculate exact index in ORCASketch to increment
        int sketch_index = (bucket_size * bucket_index) + bucket_counter_index;
        cout << "sketch_index: " << sketch_index << "\n";
        orca_sketch[sketch_index]++;
    }

    // test code for printing - TODO: delete
    cout << "\norca_sketch: ";
    for (int j = 0; j < sketch_size; j++)
    {
        if (j % bucket_size == 0)
        {
            cout << "| ";
        }
        cout << orca_sketch[j] << " ";
    }
    cout << "\n";
}

// TODO
// hash to get bucket index, and all bucket counter indexes
uint64_t ORCASketch::query(const char * str)
{
    return 0;
}

int ORCASketch::factorial(int n)
{
    int result = 1;
    for (int i = 2; i < (n+1); i++)
    {
        result = result * i;
    }
    return result;
}

int ORCASketch::combination()
{
    return factorial(bucket_size) / (factorial(number_of_bucket_counters) * factorial(bucket_size - number_of_bucket_counters));
}

// Lookup table (of counter combinations)
int **ORCASketch::create_bucket_counter_lookup_table()
{
    int** lookup_table = new int*[number_of_options];

    for (int i = 0; i < number_of_options; i++)
    {
        lookup_table[i] = new int[number_of_bucket_counters]();
    }

    // run python script to generate lookup table
    FILE* file;
    char py_file_name[] = "lookup_table.py";

    int py_argc = 3;
    char *py_argv[3];
    py_argv[0] = py_file_name;
    py_argv[1] = py_bucket_size;
    py_argv[2] = py_number_of_bucket_counters;
    // int* bucket_size_ptr = &bucket_size;
    // py_argv[1] = reinterpret_cast<char*>(bucket_size_ptr);
    // int* number_of_bucket_counters_ptr = &number_of_bucket_counters;
    // py_argv[2] = reinterpret_cast<char*>(number_of_bucket_counters_ptr);

    Py_SetProgramName(py_argv[0]);
    Py_Initialize();
    PySys_SetArgv(py_argc, py_argv);
    file = fopen(py_file_name, "r");
    PyRun_SimpleFile(file, py_file_name);
    Py_Finalize();

    // load combinations into lookup_table
    char lookup_table_file_name[] = "lookup_table.txt";
    ifstream f(lookup_table_file_name);

    char num;
    int int_num = -1;

    for (int i = 0; i < number_of_options; i++)
    {
        for (int j = 0; j < number_of_bucket_counters; j++)
        {
            while (int_num < 0)
            {
                num = f.get();
                int_num = num - 48;
            }
            lookup_table[i][j] = int_num;
            int_num = -1;
        }
    }

    return lookup_table;
}
