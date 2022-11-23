#include <iostream>

#include "salsa-src/genzipf.h"

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

    if (argc < 4) {
        cout << "Usage Error:\n";
        cout << "argv[1]: int N\n";
        cout << "argv[2]: int Seed\n";
        cout << "argv[3]: float alpha\n";
        system("pause");
		return 0;
    }

    int N = atoi(argv[1]); // largest: 99000000
    int Seed = atoi(argv[2]); // 42
    float alpha = atof(argv[3]); // e.g. 0.6, 0.8, 1, 1.2, 1.5

    if (alpha > 0) {
        char path[] = "./zipf";
        genzipf(path, Seed, alpha, 1 << 24, N);
        cout << "\nzipf generation complete!\n";
    }

    

    return 0;
}
