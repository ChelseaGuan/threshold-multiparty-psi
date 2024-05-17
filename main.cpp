#include <iostream>
#include <vector>
#include "psi_protocols.h"
#include "benchmarking.h"
// #include "NTL/BasicThreadPool.h"


// TODO: Allow variable set sizes?
int main() {
    // SetNumThreads(8);

    Keys keys;
    key_gen(&keys, 1024, 2, 3);
    std::cout << "Running benchmarks for T-MPSI (without simulated delays) using a 1024-bit key:" << std::endl;
    threshold_benchmark(std::vector<long>({5, 7, 9}), std::vector<long>({5, 7}));


    return 0;
}
