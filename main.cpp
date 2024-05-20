#include <chrono>
#include <iostream>
#include <vector>
#include <future>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <nlohmann/json.hpp>  // Include JSON library
#include "psi_protocols.h"

using json = nlohmann::json;

namespace fs = std::filesystem;

std::vector<std::vector<long>> read_directory_and_format(const std::string& directory_path) {
    std::vector<std::vector<long>> sets;

    // Iterate through all the files in the directory
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        std::vector<long> current_set;
        std::ifstream file(entry.path()); // Open the file
        std::string line;

        // Read each line in the file into a vector (one number per line)
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            long number;
            if (ss >> number) {
                current_set.push_back(number);
            }
        }

        file.close();
        sets.push_back(current_set);
    }

    return sets;
}

int main() {
    auto begin = std::chrono::high_resolution_clock::now();

    Keys keys;
    key_gen(&keys, 1024, 2, 3);

    std::vector<std::vector<long>> all_sets = read_directory_and_format("../elements");

    std::vector<std::vector<long>> results(all_sets.size());

    #pragma omp parallel for
    for (int i = 0; i < all_sets.size(); ++i) {
        std::vector<std::vector<long>> client_sets;
        for (int j = 0; j < all_sets.size(); ++j) {
            if (j != i) {
                client_sets.push_back(all_sets[j]);
            }
        }
        results[i] = threshold_multiparty_psi(client_sets, all_sets[i], 2, 16, 4, 1, keys);
    }

    // Create a JSON object
    json j;
    j["results"] = json::array();  // Create a JSON array to store results

    for (int i = 0; i < results.size(); ++i) {
        j["results"].push_back(results[i]);  // Store each result in the JSON array
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << "\tTotal time: " << ms << " milliseconds" << std::endl;

    // Write output to a JSON file
    std::ofstream file("tmpsi_ind.json");
    file << j.dump(4);  // Pretty print to file
    file.close();

    return 0;
}


//#include <iostream>
//#include <vector>
//#include "psi_protocols.h"
//#include "benchmarking.h"
//// #include "NTL/BasicThreadPool.h"
//
//
//int main() {
//    std::cout << "Running benchmarks for T-MPSI (without simulated delays) using a 1024-bit key:" << std::endl;
//    threshold_benchmark(std::vector<long>({5, 7, 9}), std::vector<long>({5, 7}));
//    return 0;
//}
