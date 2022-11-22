using namespace std;

int main(int argc, char* argv[]) {

    // no SIMD version
    // ===============
    //
    // data structure:
    // SUMMARY - one row of counters, split into buckets
    // let W = total number of counters (in the row)
    // let B = size of a bucket (in number of counters)
    // therefore, number of buckets = W / B
    // let A = number of counters an item should be mapped into within a bucket
    // therefore, number of options for which A counters within a bucket an item should be mapped into
    //            = number of rows in lookup table (of counter combinations)
    //            = B choose A (combination) 
    //
    // algorithm:
    // 1. Use BobHash (the hash function) on an item -> returns 32 bits
    // 2. Split into 22 bits and 10 (least significant) bits
    // 3. Use the 10 bits to map into a bucket
    // 4. Use the 22 bits to map into a row in the lookup table



    return 0;
}
