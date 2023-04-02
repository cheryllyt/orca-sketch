#include "ORCAS.hpp"

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
    delete[] bucket_counter_sign_lookup_table;
    delete[] orca_sketch;
}

void ORCASketch::initialize(int sketch_size, int number_of_buckets, int number_of_bucket_counters, int seed)
{
    this->sketch_size = sketch_size;
    this->number_of_buckets = number_of_buckets;
    this->number_of_bucket_counters = number_of_bucket_counters;

    assert(number_of_bucket_counters > 0);
    bool n_bucket_counter_is_pow_2 = (number_of_bucket_counters > 0) && ((number_of_bucket_counters & (number_of_bucket_counters - 1)) == 0);
    set_option_row_size(n_bucket_counter_is_pow_2); // option row size needs to be a power of 2
    option_row_size_bits = __builtin_ctz(option_row_size);

    // fixed assumption that number_of_buckets is power of 2
    assert((number_of_buckets > 0) && ((number_of_buckets & (number_of_buckets - 1)) == 0));
    number_of_bits_bucket = __builtin_ctz(number_of_buckets);

    bucket_size = sketch_size / number_of_buckets;
    bucket_mask = number_of_buckets - 1;

    // fixed assumption that bucket_size is power of 2
    assert((bucket_size > 0) && ((bucket_size & (bucket_size - 1)) == 0));
    number_of_bits_bucket_size = __builtin_ctz(bucket_size);

    set_number_of_lookup_table_options();
    number_of_options_ind = number_of_options - 1;
    create_lookup_tables(n_bucket_counter_is_pow_2);

    #ifdef DEBUG
    cout << "\nsketch_size: " << sketch_size << "\n";
    cout << "number_of_buckets: " << number_of_buckets << "\n";
    cout << "bucket_mask: " << bucket_mask << "\n";
    cout << "bucket_size: " << bucket_size << "\n";
    cout << "number_of_bucket_counters: " << number_of_bucket_counters << "\n";
    cout << "number_of_options: " << number_of_options << "\n";
    cout << "number_of_options_ind: " << number_of_options_ind << "\n";
    cout << "option_row_size: " << option_row_size << "\n";
    cout << "option_row_size_bits: " << option_row_size_bits << "\n";
    
    cout << "\n";
    for (int i = 0; i < number_of_options; i++)
    {
        int temp[bucket_size];
        _mm256_storeu_si256((__m256i*) temp, bucket_counter_vec_lookup_table[i]);
        for (int j = 0; j < bucket_size; j++)
        {
            cout << temp[j] << " ";
        }
        cout << ": " << i << "\n";
    }

    cout << "\n";
    for (int k = 0; k < (number_of_options * option_row_size); k++)
    {
        cout << (int) bucket_counter_ind_lookup_table[k] << ":" << (int) bucket_counter_sign_lookup_table[k] << " | ";
        if ((k + 1) % option_row_size == 0)
        {
            cout << "\n";
        }
    }
    #endif

    orca_sketch = new int32_t[sketch_size]();

    bobhash.initialize(seed*(7) + 100);
}

void ORCASketch::increment(const char * str)
{
    uint bobhash_return = (bobhash.run(str, FT_SIZE));

    uint bucket_index = bobhash_return & bucket_mask;
    uint option_index = 0;
    if (number_of_options_ind != 0)
    {
        option_index = (bobhash_return >> number_of_bits_bucket) % number_of_options_ind;
    }

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "option_index: " << option_index << "\n";
    #endif

    uint exact_bucket_index = bucket_index << number_of_bits_bucket_size;

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

    uint bucket_index = bobhash_return & bucket_mask;
    uint option_index = 0;
    if (number_of_options_ind != 0)
    {
        option_index = (bobhash_return >> number_of_bits_bucket) % number_of_options_ind;
    }

    uint exact_bucket_index = bucket_index << number_of_bits_bucket_size;
    uint start_option_index = option_index << option_row_size_bits;

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "option_index: " << option_index << "\n";
    cout << "exact_bucket_index: " << exact_bucket_index << "\n";
    cout << "start_option_index: " << start_option_index << "\n";
    #endif

    int32_t counter_values[number_of_bucket_counters];

    for (int i = 0; i < number_of_bucket_counters; i++)
    {
        int table_option_index = start_option_index + i;
        int counter_index = bucket_counter_ind_lookup_table[table_option_index];
        int sketch_index = exact_bucket_index + counter_index;

        int32_t counter_value = orca_sketch[sketch_index] * bucket_counter_sign_lookup_table[table_option_index];
        counter_values[i] = counter_value > 0 ? counter_value : 0;

        #ifdef DEBUG
        cout << "counter_index " << i << ": " << counter_index << "\n";
        cout << "counter_value: " << counter_value * bucket_counter_sign_lookup_table[table_option_index] << "\n";
        cout << "sign_value: " << (int) bucket_counter_sign_lookup_table[table_option_index] << "\n";
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

    if (number_of_bucket_counters == 1)
    {
        cout << "counter value / median: " << counter_values[0] << "\n";
    }
    else if (number_of_bucket_counters == 3)
    {
        cout << "counter values: " << counter_values[0] << " " << counter_values[1] << " " << counter_values[2] << "\n";
        cout << "median: " << max(min(counter_values[0], counter_values[1]), min(max(counter_values[0], counter_values[1]), counter_values[2])) << "\n";
    }
    else if (number_of_bucket_counters == 5)
    {
        cout << "counter values: " << counter_values[0] << " " << counter_values[1] << " " << counter_values[2] << " " << counter_values[3] << " " << counter_values[4] << "\n";
        cout << "median: " << (counter_values[1] < counter_values[0] ? counter_values[3] < counter_values[2] ? counter_values[1] < counter_values[3] ? counter_values[0] < counter_values[4] ? counter_values[0] < counter_values[3] ? counter_values[4] < counter_values[3] ? counter_values[4] : counter_values[3]
		 : counter_values[2] < counter_values[0] ? counter_values[2] : counter_values[0]
		 : counter_values[4] < counter_values[3] ? counter_values[0] < counter_values[3] ? counter_values[0] : counter_values[3]
		 : counter_values[2] < counter_values[4] ? counter_values[2] : counter_values[4]
		 : counter_values[2] < counter_values[4] ? counter_values[1] < counter_values[2] ? counter_values[0] < counter_values[2] ? counter_values[0] : counter_values[2]
		 : counter_values[4] < counter_values[1] ? counter_values[4] : counter_values[1]
		 : counter_values[1] < counter_values[4] ? counter_values[0] < counter_values[4] ? counter_values[0] : counter_values[4]
		 : counter_values[2] < counter_values[1] ? counter_values[2] : counter_values[1]
		 : counter_values[1] < counter_values[2] ? counter_values[0] < counter_values[4] ? counter_values[0] < counter_values[2] ? counter_values[4] < counter_values[2] ? counter_values[4] : counter_values[2]
		 : counter_values[3] < counter_values[0] ? counter_values[3] : counter_values[0]
		 : counter_values[4] < counter_values[2] ? counter_values[0] < counter_values[2] ? counter_values[0] : counter_values[2]
		 : counter_values[3] < counter_values[4] ? counter_values[3] : counter_values[4]
		 : counter_values[3] < counter_values[4] ? counter_values[1] < counter_values[3] ? counter_values[0] < counter_values[3] ? counter_values[0] : counter_values[3]
		 : counter_values[4] < counter_values[1] ? counter_values[4] : counter_values[1]
		 : counter_values[1] < counter_values[4] ? counter_values[0] < counter_values[4] ? counter_values[0] : counter_values[4]
		 : counter_values[3] < counter_values[1] ? counter_values[3] : counter_values[1]
		 : counter_values[3] < counter_values[2] ? counter_values[0] < counter_values[3] ? counter_values[1] < counter_values[4] ? counter_values[1] < counter_values[3] ? counter_values[4] < counter_values[3] ? counter_values[4] : counter_values[3]
		 : counter_values[2] < counter_values[1] ? counter_values[2] : counter_values[1]
		 : counter_values[4] < counter_values[3] ? counter_values[1] < counter_values[3] ? counter_values[1] : counter_values[3]
		 : counter_values[2] < counter_values[4] ? counter_values[2] : counter_values[4]
		 : counter_values[2] < counter_values[4] ? counter_values[0] < counter_values[2] ? counter_values[1] < counter_values[2] ? counter_values[1] : counter_values[2]
		 : counter_values[4] < counter_values[0] ? counter_values[4] : counter_values[0]
		 : counter_values[0] < counter_values[4] ? counter_values[1] < counter_values[4] ? counter_values[1] : counter_values[4]
		 : counter_values[2] < counter_values[0] ? counter_values[2] : counter_values[0]
		 : counter_values[0] < counter_values[2] ? counter_values[1] < counter_values[4] ? counter_values[1] < counter_values[2] ? counter_values[4] < counter_values[2] ? counter_values[4] : counter_values[2]
		 : counter_values[3] < counter_values[1] ? counter_values[3] : counter_values[1]
		 : counter_values[4] < counter_values[2] ? counter_values[1] < counter_values[2] ? counter_values[1] : counter_values[2]
		 : counter_values[3] < counter_values[4] ? counter_values[3] : counter_values[4]
		 : counter_values[3] < counter_values[4] ? counter_values[0] < counter_values[3] ? counter_values[1] < counter_values[3] ? counter_values[1] : counter_values[3]
		 : counter_values[4] < counter_values[0] ? counter_values[4] : counter_values[0]
		 : counter_values[0] < counter_values[4] ? counter_values[1] < counter_values[4] ? counter_values[1] : counter_values[4]
		 : counter_values[3] < counter_values[0] ? counter_values[3] : counter_values[0]) << "\n";
    }
    #endif

    // hardcoded median computations for efficiency
    #ifdef COUNTERONE
    return counter_values[0];

    // median code referenced from: https://github.com/SALSA-ICDE2021/SALSA
    #elif defined COUNTERFIVE
    return counter_values[1] < counter_values[0] ? counter_values[3] < counter_values[2] ? counter_values[1] < counter_values[3] ? counter_values[0] < counter_values[4] ? counter_values[0] < counter_values[3] ? counter_values[4] < counter_values[3] ? counter_values[4] : counter_values[3]
		 : counter_values[2] < counter_values[0] ? counter_values[2] : counter_values[0]
		 : counter_values[4] < counter_values[3] ? counter_values[0] < counter_values[3] ? counter_values[0] : counter_values[3]
		 : counter_values[2] < counter_values[4] ? counter_values[2] : counter_values[4]
		 : counter_values[2] < counter_values[4] ? counter_values[1] < counter_values[2] ? counter_values[0] < counter_values[2] ? counter_values[0] : counter_values[2]
		 : counter_values[4] < counter_values[1] ? counter_values[4] : counter_values[1]
		 : counter_values[1] < counter_values[4] ? counter_values[0] < counter_values[4] ? counter_values[0] : counter_values[4]
		 : counter_values[2] < counter_values[1] ? counter_values[2] : counter_values[1]
		 : counter_values[1] < counter_values[2] ? counter_values[0] < counter_values[4] ? counter_values[0] < counter_values[2] ? counter_values[4] < counter_values[2] ? counter_values[4] : counter_values[2]
		 : counter_values[3] < counter_values[0] ? counter_values[3] : counter_values[0]
		 : counter_values[4] < counter_values[2] ? counter_values[0] < counter_values[2] ? counter_values[0] : counter_values[2]
		 : counter_values[3] < counter_values[4] ? counter_values[3] : counter_values[4]
		 : counter_values[3] < counter_values[4] ? counter_values[1] < counter_values[3] ? counter_values[0] < counter_values[3] ? counter_values[0] : counter_values[3]
		 : counter_values[4] < counter_values[1] ? counter_values[4] : counter_values[1]
		 : counter_values[1] < counter_values[4] ? counter_values[0] < counter_values[4] ? counter_values[0] : counter_values[4]
		 : counter_values[3] < counter_values[1] ? counter_values[3] : counter_values[1]
		 : counter_values[3] < counter_values[2] ? counter_values[0] < counter_values[3] ? counter_values[1] < counter_values[4] ? counter_values[1] < counter_values[3] ? counter_values[4] < counter_values[3] ? counter_values[4] : counter_values[3]
		 : counter_values[2] < counter_values[1] ? counter_values[2] : counter_values[1]
		 : counter_values[4] < counter_values[3] ? counter_values[1] < counter_values[3] ? counter_values[1] : counter_values[3]
		 : counter_values[2] < counter_values[4] ? counter_values[2] : counter_values[4]
		 : counter_values[2] < counter_values[4] ? counter_values[0] < counter_values[2] ? counter_values[1] < counter_values[2] ? counter_values[1] : counter_values[2]
		 : counter_values[4] < counter_values[0] ? counter_values[4] : counter_values[0]
		 : counter_values[0] < counter_values[4] ? counter_values[1] < counter_values[4] ? counter_values[1] : counter_values[4]
		 : counter_values[2] < counter_values[0] ? counter_values[2] : counter_values[0]
		 : counter_values[0] < counter_values[2] ? counter_values[1] < counter_values[4] ? counter_values[1] < counter_values[2] ? counter_values[4] < counter_values[2] ? counter_values[4] : counter_values[2]
		 : counter_values[3] < counter_values[1] ? counter_values[3] : counter_values[1]
		 : counter_values[4] < counter_values[2] ? counter_values[1] < counter_values[2] ? counter_values[1] : counter_values[2]
		 : counter_values[3] < counter_values[4] ? counter_values[3] : counter_values[4]
		 : counter_values[3] < counter_values[4] ? counter_values[0] < counter_values[3] ? counter_values[1] < counter_values[3] ? counter_values[1] : counter_values[3]
		 : counter_values[4] < counter_values[0] ? counter_values[4] : counter_values[0]
		 : counter_values[0] < counter_values[4] ? counter_values[1] < counter_values[4] ? counter_values[1] : counter_values[4]
		 : counter_values[3] < counter_values[0] ? counter_values[3] : counter_values[0];

    #else // default
    return max(min(counter_values[0], counter_values[1]), min(max(counter_values[0], counter_values[1]), counter_values[2]));

    #endif
}

void ORCASketch::set_option_row_size(bool n_bucket_counter_is_pow_2)
{    
    // directly set option_row_size as number_of_bucket_counters
    if (n_bucket_counter_is_pow_2)
    {
        option_row_size = number_of_bucket_counters;
    }
    else
    {
        int n_leading_zeros = __builtin_clz(number_of_bucket_counters);
        int n_bits_bucket_counters = 32 - n_leading_zeros;
        option_row_size = pow(2, n_bits_bucket_counters);
    }
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
void ORCASketch::create_lookup_tables(bool n_bucket_counter_is_pow_2)
{
    int ind_lookup_table_len = number_of_options * option_row_size;
    
    bucket_counter_vec_lookup_table = new __m256i[number_of_options];
    bucket_counter_ind_lookup_table = new uint8_t[ind_lookup_table_len];
    bucket_counter_sign_lookup_table = new int8_t[ind_lookup_table_len];

    int temp_ind_lookup_table_len = number_of_options * number_of_bucket_counters;
    uint8_t *temp_bucket_counter_ind_lookup_table = new uint8_t[temp_ind_lookup_table_len];
    int8_t *temp_bucket_counter_sign_lookup_table = new int8_t[temp_ind_lookup_table_len];

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
            temp[j] = int_num == 9 ? -1 : int_num;
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

    // load temporary index lookup table
    for (int k = 0; k < temp_ind_lookup_table_len; k++)
    {
        while (int_num < 0) // ignore non-integer
        {
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
        } // integer found

        string index = "";

        while (int_num >= 0)
        {
            index = index + num;
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
        } // non-integer found; end of index

        temp_bucket_counter_ind_lookup_table[k] = stoi(index);
    }

    // load temporary sign lookup table
    for (int k = 0; k < temp_ind_lookup_table_len; k++)
    {
        while (int_num < 0) // ignore non-integer
        {
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
        } // integer found

        string index = "";

        while (int_num >= 0)
        {
            index = index + num;
            num = f.get();
            int_num = num - CHAR_TO_INT_DIFF;
        } // non-integer found; end of sign

        int8_t sign = stoi(index);

        temp_bucket_counter_sign_lookup_table[k] = sign == 9 ? -1 : sign;
    }

    // move index and sign into actual lookup table
    if (n_bucket_counter_is_pow_2)
    {
        for (int m = 0; m < ind_lookup_table_len; m++)
        {
            bucket_counter_ind_lookup_table[m] = temp_bucket_counter_ind_lookup_table[m];
            bucket_counter_sign_lookup_table[m] = temp_bucket_counter_sign_lookup_table[m];
        }
    }
    else
    {
        int index_to_store = 0;
        for (int option = 0; option < number_of_options; option++)
        {
            uint start_option_index = option << option_row_size_bits;
            for (int c = 0; c < number_of_bucket_counters; c++)
            {
                bucket_counter_ind_lookup_table[start_option_index + c] = temp_bucket_counter_ind_lookup_table[index_to_store];
                bucket_counter_sign_lookup_table[start_option_index + c] = temp_bucket_counter_sign_lookup_table[index_to_store];
                index_to_store++;
            }
            for (int d = number_of_bucket_counters; d < option_row_size; d++)
            {
                bucket_counter_ind_lookup_table[start_option_index + d] = -1;
                bucket_counter_sign_lookup_table[start_option_index + d] = 0;
            }
        }
    }
}
