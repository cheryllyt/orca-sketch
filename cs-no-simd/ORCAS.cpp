#include "ORCAS.hpp"

#include "Defs.hpp"

using namespace std;

template<int number_of_options_ind>
ORCASketch<number_of_options_ind>::ORCASketch()
{
}

template<int number_of_options_ind>
ORCASketch<number_of_options_ind>::~ORCASketch()
{
    delete[] array_counter_ind_lookup_table;
    delete[] array_counter_sign_lookup_table;
    delete[] orca_sketch;
}

template<int number_of_options_ind>
void ORCASketch<number_of_options_ind>::initialize(int sketch_size, int number_of_arrays, int number_of_array_counters, int seed)
{
    this->sketch_size = sketch_size;
    this->number_of_arrays = number_of_arrays;
    this->number_of_array_counters = number_of_array_counters;

    assert(number_of_array_counters > 0);
    bool n_array_counter_is_pow_2 = (number_of_array_counters > 0) && ((number_of_array_counters & (number_of_array_counters - 1)) == 0);
    set_option_row_size(n_array_counter_is_pow_2); // option row size needs to be a power of 2
    option_row_size_bits = __builtin_ctz(option_row_size);

    // fixed assumption that number_of_arrays is power of 2
    assert((number_of_arrays > 0) && ((number_of_arrays & (number_of_arrays - 1)) == 0));
    number_of_bits_array = __builtin_ctz(number_of_arrays);

    array_size = sketch_size / number_of_arrays;
    array_mask = number_of_arrays - 1;

    // fixed assumption that array_size is power of 2
    assert((array_size > 0) && ((array_size & (array_size - 1)) == 0));
    number_of_bits_array_size = __builtin_ctz(array_size);

    set_number_of_lookup_table_options();
    assert(number_of_options_ind == (number_of_options - 1));
    create_lookup_tables(n_array_counter_is_pow_2);

    #ifdef DEBUG
    cout << "\nsketch_size: " << sketch_size << "\n";
    cout << "number_of_arrays: " << number_of_arrays << "\n";
    cout << "array_mask: " << array_mask << "\n";
    cout << "array_size: " << array_size << "\n";
    cout << "number_of_array_counters: " << number_of_array_counters << "\n";
    cout << "number_of_options: " << number_of_options << "\n";
    cout << "number_of_options_ind: " << number_of_options_ind << "\n";
    cout << "option_row_size: " << option_row_size << "\n";
    cout << "option_row_size_bits: " << option_row_size_bits << "\n";
    
    cout << "\n";
    int count = 0;
    for (int k = 0; k < (number_of_options * option_row_size); k++)
    {
        cout << (int) array_counter_ind_lookup_table[k] << ":" << (int) array_counter_sign_lookup_table[k] << " | ";
        if ((k + 1) % option_row_size == 0)
        {
            cout << ": " << count << "\n";
            count++;
        }
    }
    #endif

    orca_sketch = new int32_t[sketch_size]();

    bobhash.initialize(seed*(7) + 100);
}

template<int number_of_options_ind>
void ORCASketch<number_of_options_ind>::increment(const char * str)
{
    uint bobhash_return = (bobhash.run(str, FT_SIZE));

    uint array_index = bobhash_return & array_mask;
    uint option_index = 0;
    if (number_of_options_ind != 0)
    {
        option_index = (bobhash_return >> number_of_bits_array) % number_of_options_ind;
    }

    uint exact_array_index = array_index << number_of_bits_array_size;
    uint start_option_index = option_index << option_row_size_bits;

    #ifdef DEBUG
    cout << "\narray_index: " << array_index << "\n";
    cout << "option_index: " << option_index << "\n";
    cout << "exact_array_index: " << exact_array_index << "\n";
    cout << "start_option_index: " << start_option_index << "\n";
    #endif

    for (int i = 0; i < number_of_array_counters; i++)
    {
        int table_option_index = start_option_index + i;
        int counter_index = array_counter_ind_lookup_table[table_option_index];
        int sketch_index = exact_array_index + counter_index;

        orca_sketch[sketch_index] += array_counter_sign_lookup_table[table_option_index];
    }

    #ifdef DEBUG
    cout << "\norca_sketch: ";
    for (int j = 0; j < sketch_size; j++)
    {
        if (j % array_size == 0)
        {
            cout << "| ";
        }
        cout << orca_sketch[j] << " ";
    }
    cout << "\n";
    #endif
}

template<int number_of_options_ind>
uint32_t ORCASketch<number_of_options_ind>::query(const char * str)
{
    uint bobhash_return = (bobhash.run(str, FT_SIZE));

    uint array_index = bobhash_return & array_mask;
    uint option_index = 0;
    if (number_of_options_ind != 0)
    {
        option_index = (bobhash_return >> number_of_bits_array) % number_of_options_ind;
    }

    uint exact_array_index = array_index << number_of_bits_array_size;
    uint start_option_index = option_index << option_row_size_bits;

    #ifdef DEBUG
    cout << "\narray_index: " << array_index << "\n";
    cout << "option_index: " << option_index << "\n";
    cout << "exact_array_index: " << exact_array_index << "\n";
    cout << "start_option_index: " << start_option_index << "\n";
    #endif

    int32_t counter_values[number_of_array_counters];

    for (int i = 0; i < number_of_array_counters; i++)
    {
        int table_option_index = start_option_index + i;
        int counter_index = array_counter_ind_lookup_table[table_option_index];
        int sketch_index = exact_array_index + counter_index;

        int32_t counter_value = orca_sketch[sketch_index] * array_counter_sign_lookup_table[table_option_index];
        counter_values[i] = counter_value > 0 ? counter_value : 0;

        #ifdef DEBUG
        cout << "counter_index " << i << ": " << counter_index << "\n";
        cout << "counter_value: " << counter_value * array_counter_sign_lookup_table[table_option_index] << "\n";
        cout << "sign_value: " << (int) array_counter_sign_lookup_table[table_option_index] << "\n";
        #endif
    }

    #ifdef DEBUG
    cout << "\norca_sketch for query: ";
    for (int j = 0; j < sketch_size; j++)
    {
        if (j % array_size == 0)
        {
            cout << "| ";
        }
        cout << orca_sketch[j] << " ";
    }
    cout << "\n";

    if (number_of_array_counters == 1)
    {
        cout << "counter value / median: " << counter_values[0] << "\n";
    }
    else if (number_of_array_counters == 3)
    {
        cout << "counter values: " << counter_values[0] << " " << counter_values[1] << " " << counter_values[2] << "\n";
        cout << "median: " << max(min(counter_values[0], counter_values[1]), min(max(counter_values[0], counter_values[1]), counter_values[2])) << "\n";
    }
    else if (number_of_array_counters == 5)
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

template<int number_of_options_ind>
void ORCASketch<number_of_options_ind>::set_option_row_size(bool n_array_counter_is_pow_2)
{    
    // directly set option_row_size as number_of_array_counters
    if (n_array_counter_is_pow_2)
    {
        option_row_size = number_of_array_counters;
    }
    else
    {
        int n_leading_zeros = __builtin_clz(number_of_array_counters);
        int n_bits_array_counters = 32 - n_leading_zeros;
        option_row_size = pow(2, n_bits_array_counters);
    }
}

template<int number_of_options_ind>
void ORCASketch<number_of_options_ind>::set_number_of_lookup_table_options()
{
    // run python script to generate lookup table
    char py_file_name[] = "lookup_table.py";

    int py_argc = 3;
    char *py_argv[3];
    py_argv[0] = py_file_name;

    int array_size_len = to_string(array_size).length();
    char* array_size_char = new char[array_size_len];
    to_chars(array_size_char, array_size_char + array_size_len, array_size);
    py_argv[1] = array_size_char;

    int number_of_array_counters_len = to_string(number_of_array_counters).length();
    char* number_of_array_counters_char = new char[number_of_array_counters_len];
    to_chars(number_of_array_counters_char, number_of_array_counters_char + number_of_array_counters_len, number_of_array_counters);
    py_argv[2] = number_of_array_counters_char;

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
template<int number_of_options_ind>
void ORCASketch<number_of_options_ind>::create_lookup_tables(bool n_array_counter_is_pow_2)
{
    int ind_lookup_table_len = number_of_options * option_row_size;
    
    array_counter_ind_lookup_table = new uint8_t[ind_lookup_table_len];
    array_counter_sign_lookup_table = new int8_t[ind_lookup_table_len];

    int temp_ind_lookup_table_len = number_of_options * number_of_array_counters;
    uint8_t *temp_array_counter_ind_lookup_table = new uint8_t[temp_ind_lookup_table_len];
    int8_t *temp_array_counter_sign_lookup_table = new int8_t[temp_ind_lookup_table_len];

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

        temp_array_counter_ind_lookup_table[k] = stoi(index);
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

        temp_array_counter_sign_lookup_table[k] = sign == 9 ? -1 : sign;
    }

    // move index and sign into actual lookup table
    if (n_array_counter_is_pow_2)
    {
        for (int m = 0; m < ind_lookup_table_len; m++)
        {
            array_counter_ind_lookup_table[m] = temp_array_counter_ind_lookup_table[m];
            array_counter_sign_lookup_table[m] = temp_array_counter_sign_lookup_table[m];
        }
    }
    else
    {
        int index_to_store = 0;
        for (int option = 0; option < number_of_options; option++)
        {
            uint start_option_index = option << option_row_size_bits;
            for (int c = 0; c < number_of_array_counters; c++)
            {
                array_counter_ind_lookup_table[start_option_index + c] = temp_array_counter_ind_lookup_table[index_to_store];
                array_counter_sign_lookup_table[start_option_index + c] = temp_array_counter_sign_lookup_table[index_to_store];
                index_to_store++;
            }
            for (int d = number_of_array_counters; d < option_row_size; d++)
            {
                array_counter_ind_lookup_table[start_option_index + d] = -1;
                array_counter_sign_lookup_table[start_option_index + d] = 0;
            }
        }
    }
}

// template explicit instantiations
template class ORCASketch<OPTION_64C3>;
template class ORCASketch<OPTION_32C3>;
template class ORCASketch<OPTION_8C3>;

template class ORCASketch<OPTION_16C1_4C3>;
template class ORCASketch<OPTION_16C3>;
template class ORCASketch<OPTION_16C5>;
