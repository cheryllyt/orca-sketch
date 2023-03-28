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
    uint exact_bucket_index = bucket_size * bucket_index;

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "exact_bucket_index: " << exact_bucket_index << "\n";
    #endif
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index = (bobhash[i].run(str, FT_SIZE)) & bucket_counter_mask;
        int sketch_index = exact_bucket_index + bucket_counter_index; // calculate exact index in ORCASketch to increment
        
        orca_sketch[sketch_index]++;

        #ifdef DEBUG
        cout << "bucket_counter_index: " << bucket_counter_index << "\n";
        cout << "sketch_index: " << sketch_index << "\n";
        #endif
    }

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
    uint bucket_index = (bobhash[BUCKET_HASH].run(str, FT_SIZE)) & bucket_number_mask;
    uint exact_bucket_index = bucket_size * bucket_index;

    #ifdef DEBUG
    cout << "\nbucket_index: " << bucket_index << "\n";
    cout << "exact_bucket_index: " << exact_bucket_index << "\n";
    #endif

    uint32_t min = UINT32_MAX;
    
    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index = (bobhash[i].run(str, FT_SIZE)) & bucket_counter_mask;
        int sketch_index = exact_bucket_index + bucket_counter_index; // calculate exact index in ORCASketch to increment
        
        uint32_t value = orca_sketch[sketch_index]; // value stored in counter
        if (value < min)
        {
            min = value;
        }

        #ifdef DEBUG
        cout << "bucket_counter_index " << i << ": " << bucket_counter_index << "\n";
        cout << "value: " << value << "\n";
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
