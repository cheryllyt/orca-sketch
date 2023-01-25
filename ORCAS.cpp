#include "ORCAS.hpp"

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
    delete[] bucket_counter_vec_lookup_table;
    delete[] bucket_counter_ind_lookup_table;
    delete[] orca_sketch;
}

void ORCASketch::initialize(int sketch_size, int number_of_buckets, int number_of_bucket_counters, int seed)
{
    this->sketch_size = sketch_size;
    this->number_of_buckets = number_of_buckets;
    this->number_of_bucket_counters = number_of_bucket_counters;

    bucket_size = sketch_size / number_of_buckets;
    bucket_mask = number_of_buckets - 1;

    set_number_of_lookup_table_options();
    option_mask = number_of_options - 1;
    create_lookup_tables();

    #ifdef DEBUG
    cout << "\n";
    for (int i = 0; i < number_of_options; i++)
    {
        int temp[bucket_size];
        _mm256_storeu_si256((__m256i*) temp, bucket_counter_vec_lookup_table[i]);
        for (int j = 0; j < bucket_size; j++)
        {
            cout << temp[j] << " ";
        }
        cout << "\n";
    }

    cout << "\n";
    for (int k = 0; k < (number_of_options * number_of_bucket_counters); k++)
    {
        cout << (int) bucket_counter_ind_lookup_table[k] << " ";
        if ((k + 1) % number_of_bucket_counters == 0)
        {
            cout << "\n";
        }
    }

    cout << "\nsketch_size: " << sketch_size << "\n";
    cout << "number_of_buckets: " << number_of_buckets << "\n";
    cout << "bucket_mask: " << bucket_mask << "\n";
    cout << "bucket_size: " << bucket_size << "\n";
    cout << "number_of_bucket_counters: " << number_of_bucket_counters << "\n";
    cout << "number_of_options: " << number_of_options << "\n";
    cout << "option_mask: " << option_mask << "\n";
    #endif

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

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "option_index: " << option_index << "\n";
    #endif

    uint exact_bucket_index = bucket_size * bucket_index;

    __m256i counter_vec = bucket_counter_vec_lookup_table[option_index];
    __m256i& orca_ptr = *((__m256i*) &orca_sketch[exact_bucket_index]);
    orca_ptr = _mm256_add_epi32(orca_ptr, counter_vec);

    #ifdef DEBUG
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
    #endif
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

    uint exact_bucket_index = bucket_size * bucket_index;
    uint start_option_index = option_index * number_of_bucket_counters;

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "option_index: " << option_index << "\n";
    cout << "exact_bucket_index: " << exact_bucket_index << "\n";
    cout << "start_option_index: " << start_option_index << "\n";
    #endif

    uint32_t min = UINT32_MAX;

    for (int i = 0; i < number_of_bucket_counters; i++)
    {
        int counter_index = bucket_counter_ind_lookup_table[start_option_index + i];
        int sketch_index = exact_bucket_index + counter_index;

        uint32_t counter_value = orca_sketch[sketch_index];
        if (counter_value < min)
        {
            min = counter_value;
        }

        #ifdef DEBUG
        cout << "counter_index " << i << ": " << counter_index << "\n";
        cout << "counter_value: " << counter_value << "\n";
        #endif
    }

    #ifdef DEBUG
    cout << "\norca_sketch for query: ";
    for (int j = 0; j < sketch_size; j++)
    {
        if (j % bucket_size == 0)
        {
            cout << "| ";
        }
        cout << orca_sketch[j] << " ";
    }
    cout << "\n";
    cout << "min: " << min << "\n";
    #endif

    return min;
}

void ORCASketch::set_number_of_lookup_table_options()
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

    number_of_options = stoi(options);
}

// Lookup table (of counter combinations)
void ORCASketch::create_lookup_tables()
{
    int ind_lookup_table_len = number_of_options * number_of_bucket_counters;
    
    bucket_counter_vec_lookup_table = new __m256i[number_of_options];
    bucket_counter_ind_lookup_table = new uint8_t[ind_lookup_table_len];

    // load combinations into lookup tables
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

    // load vector lookup table
    for (int i = 0; i < number_of_options; i++)
    {
        int temp[bucket_size];
        for (int j = 0; j < bucket_size; j++)
        {
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
            temp[j] = int_num;
        }
        // assumption that buckets are fixed at size 8
        bucket_counter_vec_lookup_table[i] = _mm256_set_epi32(temp[7], temp[6], temp[5], temp[4],
                                                              temp[3], temp[2], temp[1], temp[0]);
    }

    while (int_num >= 0)
    {
        num = f.get();
        int_num = num - CHAR_TO_INT_DIFF;
    } // whitespace found

    // load index lookup table
    for (int k = 0; k < ind_lookup_table_len; k++)
    {
        while (int_num < 0) // ignore whitespace
        {
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
        } // non-whitespace found

        string index = "";

        while (int_num >= 0)
        {
            index = index + num;
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
        } // whitespace found; end of index

        bucket_counter_ind_lookup_table[k] = stoi(index);
    }
}
