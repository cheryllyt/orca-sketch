#include "ORCAS.hpp"
#include <immintrin.h>

#define NUMBER_OF_HASH_FUNC 1
#define LEAST_SIGNIF_10 1023
#define MOST_SIGNIF_10 10
#define CHAR_TO_INT_DIFF 48
#define FT_SIZE 13

using namespace std;

ORCASketch::ORCASketch()
{
}

ORCASketch::~ORCASketch()
{
    for (int i = 0; i < number_of_options; ++i)
	{
		delete[] bucket_counter_lookup_table[i];
	}
    delete[] bucket_counter_lookup_table;
    delete[] orca_sketch;
}

void ORCASketch::initialize(int sketch_size, int number_of_buckets, int number_of_bucket_counters, int seed)
{
    this->sketch_size = sketch_size;
    this->number_of_buckets = number_of_buckets;
    this->number_of_bucket_counters = number_of_bucket_counters;

    bucket_size = sketch_size / number_of_buckets;
    bucket_mask = number_of_buckets - 1;

    number_of_options = get_number_of_lookup_table_options();
    option_mask = number_of_options - 1;
    bucket_counter_lookup_table = create_bucket_counter_lookup_table();

    // test code for printing - TODO: delete
    cout << "\n";
    for (int i = 0; i < number_of_options; i++)
    {
        for (int j = 0; j < bucket_size; j++)
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

    bobhash.initialize(seed*(7) + 100);
}

void ORCASketch::increment(const char * str)
{
    uint bobhash_return = (bobhash.run(str, FT_SIZE));

    uint bucket_index = (bobhash_return & LEAST_SIGNIF_10) & bucket_mask;
    uint option_index = 0;
    if (option_mask != 0)
    {
        option_index = (bobhash_return >> MOST_SIGNIF_10) % option_mask;
    }

    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "option_index: " << option_index << "\n";

    uint exact_bucket_index = bucket_size * bucket_index;

    // TODO: lookup table should directly contain the vectors of bucket counters
    cout << "\n" << bucket_counter_lookup_table[option_index][0] << " " <<
                                    bucket_counter_lookup_table[option_index][1] << " " <<
                                    bucket_counter_lookup_table[option_index][2] << " " <<
                                    bucket_counter_lookup_table[option_index][3] << " " <<
                                    bucket_counter_lookup_table[option_index][4] << " " <<
                                    bucket_counter_lookup_table[option_index][5] << " " <<
                                    bucket_counter_lookup_table[option_index][6] << " " <<
                                    bucket_counter_lookup_table[option_index][7] << "\n";

    // __m256i cons = _mm256_set_epi32(0,1,0,1,0,1,1,1); // increments in reverse order
    // __m256i& cons = *((__m256i*) &bucket_counter_lookup_table[option_index]);

    // right now - assumption that buckets are of size 8
    // TODO: remove hard code to allow for buckets smaller than 8
    __m256i cons = _mm256_set_epi32(bucket_counter_lookup_table[option_index][0],
                                    bucket_counter_lookup_table[option_index][1],
                                    bucket_counter_lookup_table[option_index][2],
                                    bucket_counter_lookup_table[option_index][3],
                                    bucket_counter_lookup_table[option_index][4],
                                    bucket_counter_lookup_table[option_index][5],
                                    bucket_counter_lookup_table[option_index][6],
                                    bucket_counter_lookup_table[option_index][7]);
    
    __m256i& ptr = *((__m256i*) &orca_sketch[exact_bucket_index]);
    ptr = _mm256_add_epi32(ptr, cons);

    // for (int i = 0; i < number_of_bucket_counters; i++)
    // {
    //     int counter_index = bucket_counter_lookup_table[option_index][i];

    //     // calculate exact index in ORCASketch to increment
    //     int sketch_index = exact_bucket_index + counter_index;

    //     // cout << "counter_index " << i << ": " << counter_index << "\n";
    //     // cout << "sketch_index: " << sketch_index << "\n";

    //     orca_sketch[sketch_index]++;
    // }

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

uint32_t ORCASketch::query(const char * str)
{
    uint bobhash_return = (bobhash.run(str, FT_SIZE));

    uint bucket_index = (bobhash_return & LEAST_SIGNIF_10) & bucket_mask;
    uint option_index = 0;
    if (option_mask != 0)
    {
        option_index = (bobhash_return >> MOST_SIGNIF_10) % option_mask;
    }

    // cout << "\nbucket_index: " << bucket_index << "\n";
    // cout << "option_index: " << option_index << "\n";

    uint32_t min = UINT32_MAX;

    uint exact_bucket_index = bucket_size * bucket_index;
    for (int i = 0; i < number_of_bucket_counters; i++)
    {
        int counter_index = bucket_counter_lookup_table[option_index][i];
        int sketch_index = exact_bucket_index + counter_index;

        uint32_t counter_value = orca_sketch[sketch_index];
        if (counter_value < min)
        {
            min = counter_value;
        }

        // cout << "counter_index " << i << ": " << counter_index << "\n";
        // cout << "counter_value: " << counter_value << "\n";
    }

    // test code for printing - TODO: delete
    // cout << "\norca_sketch for query: ";
    // for (int j = 0; j < sketch_size; j++)
    // {
    //     if (j % bucket_size == 0)
    //     {
    //         cout << "| ";
    //     }
    //     cout << orca_sketch[j] << " ";
    // }
    // cout << "\n";
    // cout << "min: " << min << "\n";

    return min;
}

int ORCASketch::get_number_of_lookup_table_options()
{
    // run python script to generate lookup table
    char py_file_name[] = "lookup_table.py";

    int py_argc = 3;
    char *py_argv[3];
    py_argv[0] = py_file_name;

    int bucket_size_len = to_string(bucket_size).length();
    char* bucket_size_char = new char[bucket_size_len];
    to_chars(bucket_size_char, bucket_size_char + bucket_size_len, bucket_size);
    py_argv[1] = bucket_size_char;

    int number_of_bucket_counters_len = to_string(number_of_bucket_counters).length();
    char* number_of_bucket_counters_char = new char[number_of_bucket_counters_len];
    to_chars(number_of_bucket_counters_char, number_of_bucket_counters_char + number_of_bucket_counters_len, number_of_bucket_counters);
    py_argv[2] = number_of_bucket_counters_char;

    Py_SetProgramName(py_argv[0]);
    Py_Initialize();
    PySys_SetArgv(py_argc, py_argv);
    FILE* file = fopen(py_file_name, "r");
    PyRun_SimpleFile(file, py_file_name);
    Py_Finalize();

    // return first line of lookup_table.txt
    char lookup_table_file_name[] = "lookup_table.txt";
    ifstream f(lookup_table_file_name);

    string options = "";

    char num = '0';
    int int_num = 0;

    while (int_num >= 0)
    {
        options = options + num;
        num = f.get();
        int_num = num - CHAR_TO_INT_DIFF;
    } // whitespace found

    return stoi(options);
}

// Lookup table (of counter combinations)
int **ORCASketch::create_bucket_counter_lookup_table()
{
    int** lookup_table = new int*[number_of_options];

    for (int i = 0; i < number_of_options; i++)
    {
        lookup_table[i] = new int[bucket_size]();
    }

    // load combinations into lookup_table
    char lookup_table_file_name[] = "lookup_table.txt";
    ifstream f(lookup_table_file_name);

    // remove first line of lookup_table.txt
    char num = '0';
    int int_num = 0;

    while (int_num >= 0)
    {
        num = f.get();
        int_num = num - CHAR_TO_INT_DIFF;
    } // whitespace found

    for (int i = 0; i < number_of_options; i++)
    {
        for (int j = 0; j < bucket_size; j++)
        {
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
            lookup_table[i][j] = int_num;
        }
    }

    return lookup_table;
}
