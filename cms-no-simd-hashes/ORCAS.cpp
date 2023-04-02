#include "ORCAS.hpp"

#define ARRAY_HASH 0
#define COUNTER_HASH 1
#define FT_SIZE 13

using namespace std;

ORCASketch::ORCASketch()
{
}

ORCASketch::~ORCASketch()
{
    delete[] bobhash;
    delete[] orca_sketch;
}

void ORCASketch::initialize(int sketch_size, int number_of_arrays, int number_of_array_counters, int seed)
{
    this->sketch_size = sketch_size;
    this->number_of_arrays = number_of_arrays;
    this->number_of_array_counters = number_of_array_counters;

    array_size = sketch_size / number_of_arrays;
    array_number_mask = number_of_arrays - 1;
    array_counter_mask = array_size - 1;

    // fixed assumption that array_size is power of 2
    assert((array_size > 0) && ((array_size & (array_size - 1)) == 0));
    number_of_bits_array_size = __builtin_ctz(array_size);

    number_of_hash_functions = number_of_array_counters + 1;

    #ifdef DEBUG
    cout << "\nsketch_size: " << sketch_size << "\n";
    cout << "number_of_arrays: " << number_of_arrays << "\n";
    cout << "array_number_mask: " << array_number_mask << "\n";
    cout << "array_size: " << array_size << "\n";
    cout << "number_of_array_counters: " << number_of_array_counters << "\n";
    cout << "array_counter_mask: " << array_counter_mask << "\n";
    cout << "number_of_hash_functions: " << number_of_hash_functions << "\n";
    #endif

    orca_sketch = new uint32_t[sketch_size]();

    bobhash = new BOBHash[number_of_hash_functions];
    for (int i = 0; i < number_of_hash_functions; i++)
    {
        bobhash[i].initialize(seed*(7 + i) + i + 100);
    }
}

void ORCASketch::increment(const char * str)
{
    uint array_index = (bobhash[ARRAY_HASH].run(str, FT_SIZE)) & array_number_mask;
    uint exact_array_index = array_index << number_of_bits_array_size;

    #ifdef DEBUG
    cout << "\narray_index: " << array_index << "\n";
    cout << "exact_array_index: " << exact_array_index << "\n";
    #endif

    int temp_array_counter_row_mask = array_counter_mask;

    int remaining_array_counters[array_size];
    for (int c = 0; c < array_size; c++)
    {
        remaining_array_counters[c] = c;
    }
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint array_counter_index_row = (bobhash[i].run(str, FT_SIZE)) & temp_array_counter_row_mask;
        uint array_counter_index = remaining_array_counters[array_counter_index_row];
        
        orca_sketch[exact_array_index + array_counter_index]++; // sketch index

        // reassign counters to a new row
        for (int n = array_counter_index_row; n < temp_array_counter_row_mask; n++)
        {
            remaining_array_counters[n] = remaining_array_counters[n + 1];
        }
        temp_array_counter_row_mask -= 1;

        #ifdef DEBUG
        cout << "array_counter_index_row " << i << ": " << array_counter_index_row << "\n";
        cout << "array_counter_index: " << array_counter_index << "\n";
        cout << "remaining_array_counters: ";
        for (int i = 0; i < array_size; i++)
        {
            cout << remaining_array_counters[i] << " ";
        }
        cout << "\n";
        #endif
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

uint32_t ORCASketch::query(const char * str)
{
    uint array_index = (bobhash[ARRAY_HASH].run(str, FT_SIZE)) & array_number_mask;
    uint exact_array_index = array_index << number_of_bits_array_size;

    #ifdef DEBUG
    cout << "\narray_index: " << array_index << "\n";
    cout << "exact_array_index: " << exact_array_index << "\n";
    #endif

    int temp_array_counter_row_mask = array_counter_mask;

    int remaining_array_counters[array_size];
    for (int c = 0; c < array_size; c++)
    {
        remaining_array_counters[c] = c;
    }

    uint32_t min = UINT32_MAX;
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint array_counter_index_row = (bobhash[i].run(str, FT_SIZE)) & temp_array_counter_row_mask;
        uint array_counter_index = remaining_array_counters[array_counter_index_row];
        
        uint32_t value = orca_sketch[exact_array_index + array_counter_index]; // value stored in counter
        if (value < min)
        {
            min = value;
        }

        // reassign counters to a new row
        for (int n = array_counter_index_row; n < temp_array_counter_row_mask; n++)
        {
            remaining_array_counters[n] = remaining_array_counters[n + 1];
        }
        temp_array_counter_row_mask -= 1;

        #ifdef DEBUG
        cout << "array_counter_index " << i << ": " << array_counter_index << "\n";
        cout << "value: " << value << "\n";
        cout << "remaining_array_counters: ";
        for (int i = 0; i < array_size; i++)
        {
            cout << remaining_array_counters[i] << " ";
        }
        cout << "\n";
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
    cout << "min: " << min << "\n";
    #endif

    return min;
}
