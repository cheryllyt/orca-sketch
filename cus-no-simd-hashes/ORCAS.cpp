#include "ORCAS.hpp"

#define BUCKET_HASH 0
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

void ORCASketch::initialize(int sketch_size, int number_of_buckets, int number_of_bucket_counters, int seed)
{
    this->sketch_size = sketch_size;
    this->number_of_buckets = number_of_buckets;
    this->number_of_bucket_counters = number_of_bucket_counters;

    bucket_size = sketch_size / number_of_buckets;
    bucket_number_mask = number_of_buckets - 1;
    bucket_counter_mask = bucket_size - 1;

    // fixed assumption that bucket_size is power of 2
    assert((bucket_size > 0) && ((bucket_size & (bucket_size - 1)) == 0));
    number_of_bits_bucket_size = __builtin_ctz(bucket_size);

    number_of_hash_functions = number_of_bucket_counters + 1;

    #ifdef DEBUG
    cout << "\nsketch_size: " << sketch_size << "\n";
    cout << "number_of_buckets: " << number_of_buckets << "\n";
    cout << "bucket_number_mask: " << bucket_number_mask << "\n";
    cout << "bucket_size: " << bucket_size << "\n";
    cout << "number_of_bucket_counters: " << number_of_bucket_counters << "\n";
    cout << "bucket_counter_mask: " << bucket_counter_mask << "\n";
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
    uint bucket_index = (bobhash[BUCKET_HASH].run(str, FT_SIZE)) & bucket_number_mask;
    uint exact_bucket_index = bucket_index << number_of_bits_bucket_size;

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "exact_bucket_index: " << exact_bucket_index << "\n";
    #endif

    uint32_t min = UINT32_MAX;

    // find min value
    int temp_bucket_counter_row_mask = bucket_counter_mask;

    int remaining_bucket_counters[bucket_size];
    for (int c = 0; c < bucket_size; c++)
    {
        remaining_bucket_counters[c] = c;
    }
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index_row = (bobhash[i].run(str, FT_SIZE)) & temp_bucket_counter_row_mask;
        uint bucket_counter_index = remaining_bucket_counters[bucket_counter_index_row];
        int sketch_index = exact_bucket_index + bucket_counter_index; // calculate exact index in ORCASketch to increment
        
        uint32_t counter_value = orca_sketch[sketch_index];
        if (counter_value < min)
        {
            min = counter_value;
        }

        // reassign counters to a new row
        for (int n = bucket_counter_index_row; n < temp_bucket_counter_row_mask; n++)
        {
            remaining_bucket_counters[n] = remaining_bucket_counters[n + 1];
        }
        temp_bucket_counter_row_mask -= 1;

        #ifdef DEBUG
        cout << "bucket_counter_index_row " << i << ": " << bucket_counter_index_row << "\n";
        cout << "bucket_counter_index: " << bucket_counter_index << "\n";
        cout << "counter_value: " << counter_value << "\n";
        cout << "remaining_bucket_counters: ";
        for (int i = 0; i < bucket_size; i++)
        {
            cout << remaining_bucket_counters[i] << " ";
        }
        cout << "\n";
        #endif
    }

    // increment only counters with min value
    temp_bucket_counter_row_mask = bucket_counter_mask;

    for (int c = 0; c < bucket_size; c++)
    {
        remaining_bucket_counters[c] = c;
    }
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index_row = (bobhash[i].run(str, FT_SIZE)) & temp_bucket_counter_row_mask;
        uint bucket_counter_index = remaining_bucket_counters[bucket_counter_index_row];
        int sketch_index = exact_bucket_index + bucket_counter_index; // calculate exact index in ORCASketch to increment
        
        if (orca_sketch[sketch_index] == min)
        {
            orca_sketch[sketch_index]++;
        }

        // reassign counters to a new row
        for (int n = bucket_counter_index_row; n < temp_bucket_counter_row_mask; n++)
        {
            remaining_bucket_counters[n] = remaining_bucket_counters[n + 1];
        }
        temp_bucket_counter_row_mask -= 1;
    }

    #ifdef DEBUG
    cout << "min: " << min << "\n";
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
    uint bucket_index = (bobhash[BUCKET_HASH].run(str, FT_SIZE)) & bucket_number_mask;
    uint exact_bucket_index = bucket_index << number_of_bits_bucket_size;

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "exact_bucket_index: " << exact_bucket_index << "\n";
    #endif

    int temp_bucket_counter_row_mask = bucket_counter_mask;

    int remaining_bucket_counters[bucket_size];
    for (int c = 0; c < bucket_size; c++)
    {
        remaining_bucket_counters[c] = c;
    }

    uint32_t min = UINT32_MAX;
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index_row = (bobhash[i].run(str, FT_SIZE)) & temp_bucket_counter_row_mask;
        uint bucket_counter_index = remaining_bucket_counters[bucket_counter_index_row];
        
        uint32_t value = orca_sketch[exact_bucket_index + bucket_counter_index]; // value stored in counter
        if (value < min)
        {
            min = value;
        }

        // reassign counters to a new row
        for (int n = bucket_counter_index_row; n < temp_bucket_counter_row_mask; n++)
        {
            remaining_bucket_counters[n] = remaining_bucket_counters[n + 1];
        }
        temp_bucket_counter_row_mask -= 1;

        #ifdef DEBUG
        cout << "bucket_counter_index " << i << ": " << bucket_counter_index << "\n";
        cout << "value: " << value << "\n";
        cout << "remaining_bucket_counters: ";
        for (int i = 0; i < bucket_size; i++)
        {
            cout << remaining_bucket_counters[i] << " ";
        }
        cout << "\n";
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
