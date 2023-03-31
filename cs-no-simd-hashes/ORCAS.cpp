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
    delete[] orca_sketch;
    delete[] bobhash;
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

    orca_sketch = new int32_t[sketch_size]();

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

    int temp_bucket_counter_row_mask = bucket_counter_mask;

    int remaining_bucket_counters[bucket_size];
    for (int c = 0; c < bucket_size; c++)
    {
        remaining_bucket_counters[c] = c;
    }

    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index_and_sign = bobhash[i].run(str, FT_SIZE);
        int bucket_counter_index_row = (bucket_counter_index_and_sign >> 0b1) & temp_bucket_counter_row_mask;
        uint bucket_counter_index = remaining_bucket_counters[bucket_counter_index_row];
        int sign = 1 - 2 * (bucket_counter_index_and_sign & 0b1);

        #ifdef DEBUG
        cout << "remaining_bucket_counters: ";
        for (int i = 0; i < bucket_size; i++)
        {
            cout << remaining_bucket_counters[i] << " ";
        }
        cout << "\n";
        #endif

        // reassign counters to a new row
        for (int n = bucket_counter_index_row; n < temp_bucket_counter_row_mask; n++)
        {
            remaining_bucket_counters[n] = remaining_bucket_counters[n + 1];
        }
        temp_bucket_counter_row_mask -= 1;

        orca_sketch[exact_bucket_index + bucket_counter_index] += sign; // sketch index

        #ifdef DEBUG
        cout << "bucket_counter_index_row " << i << ": " << bucket_counter_index_row << "\n";
        cout << "bucket_counter_index " << i << ": " << bucket_counter_index << "\n";
        cout << "sign_value: " << sign << "\n";
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

    int temp_bucket_counter_row_mask = bucket_counter_mask;

    int remaining_bucket_counters[bucket_size];
    for (int c = 0; c < bucket_size; c++)
    {
        remaining_bucket_counters[c] = c;
    }

    int32_t counter_values[number_of_bucket_counters];

    for (int i = COUNTER_HASH; i < number_of_hash_functions; i++)
    {
        uint bucket_counter_index_and_sign = bobhash[i].run(str, FT_SIZE);
        int bucket_counter_index_row = (bucket_counter_index_and_sign >> 0b1) & temp_bucket_counter_row_mask;
        uint bucket_counter_index = remaining_bucket_counters[bucket_counter_index_row];
        int sign = 1 - 2 * (bucket_counter_index_and_sign & 0b1);

        #ifdef DEBUG
        cout << "remaining_bucket_counters: ";
        for (int i = 0; i < bucket_size; i++)
        {
            cout << remaining_bucket_counters[i] << " ";
        }
        cout << "\n";
        #endif

        // reassign counters to a new row
        for (int n = bucket_counter_index_row; n < temp_bucket_counter_row_mask; n++)
        {
            remaining_bucket_counters[n] = remaining_bucket_counters[n + 1];
        }
        temp_bucket_counter_row_mask -= 1;

        int32_t counter_value = orca_sketch[exact_bucket_index + bucket_counter_index] * sign; // sketch index
        counter_values[i - COUNTER_HASH] = counter_value > 0 ? counter_value : 0;

        #ifdef DEBUG
        cout << "bucket_counter_index_row " << i << ": " << bucket_counter_index_row << "\n";
        cout << "bucket_counter_index " << i << ": " << bucket_counter_index << "\n";
        cout << "counter_value: " << counter_value * sign << "\n";
        cout << "sign_value: " << sign << "\n";
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
