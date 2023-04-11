#ifndef ORCAS_TESTS
#define ORCAS_TESTS

template<int number_of_options_ind>
void test_orcas_error_on_arrival(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template<int number_of_options_ind>
void test_orcas_speed(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

template<int number_of_options_ind>
void test_orcas_final_error(int N, int sketch_size, int number_of_arrays, int number_of_array_counters, int seed, const char* data);

#endif // !ORCAS_TESTS
