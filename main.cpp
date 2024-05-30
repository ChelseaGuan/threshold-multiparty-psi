#include <chrono>
#include <iostream>
#include <vector>
#include <future>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <cmath>  // For log2 function
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
    std::string path = "../benchmark";  // Path to the benchmark directory

    // Traverse the benchmark directory
    for (const auto& entry : fs::directory_iterator(path)) {
        if (!entry.is_directory()) {
            continue;
        }
        else {
            // Extract directory name
            std::string dirName = entry.path().filename().string();
            std::string mahdaviBenchmark = entry.path().filename().string();
            // Split the directory name by underscores
            std::string parts[5]; // to store each part of the directory name
            int i = 0;
            size_t pos = 0;
            std::string token;
            std::string delimiter = "_";
            while ((pos = dirName.find(delimiter)) != std::string::npos) {
                token = dirName.substr(0, pos);
                parts[i++] = token;
                dirName.erase(0, pos + delimiter.length());
            }

            // Assign variables based on the parts
            int m = std::stoi(parts[1]); // number of parties m
            int l = m / 2;  // HE threshold l
            int T = std::stoi(parts[3]); // intersection threshold T
            long exp = std::log2(std::stoi(parts[2])); // log_2 of set size

            // Output to verify
            std::cout << "Directory: " << entry.path() << std::endl;
            std::cout << "m = " << m << ", l = " << l << ", T = " << T << ", exp = " << exp << std::endl;

            auto begin = std::chrono::high_resolution_clock::now();

            long m_bits = ceil((7.0 * (1 << exp)) / log(2.0));
            long k_hashes = 7;

            Keys keys;
            key_gen(&keys, 1024, l, m);

            std::vector<std::vector<long>> all_sets = read_directory_and_format("../benchmark/" + mahdaviBenchmark + "/elements");
            std::vector<std::vector<long>> results(all_sets.size());


            #pragma omp parallel for
            for (int i = 0; i < all_sets.size(); ++i) {
                std::vector<std::vector<long>> client_sets;
                for (int j = 0; j < all_sets.size(); ++j) {
                    if (j != i) {
                        client_sets.push_back(all_sets[j]);
                    }
                }
                results[i] = threshold_multiparty_psi(client_sets, all_sets[i], l, m_bits, k_hashes, T, keys);
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto dur = end - begin;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

            std::string fileName = "Bay_Parallelized_benchmark_" + std::to_string(m) + "_" + std::to_string(lround(std::pow(2, exp))) + "_" + std::to_string(T) + ".txt";
            std::ofstream file(fileName, std::ios::app);

            // Check if file is open
            if (file.is_open()) {
                file << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Bay Parallelized <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
                file << "Total Time: " << ms << " ms\n\nResults: " << std::endl;
                for (int i = 0; i < results.size(); ++i) {
                    file << i << ":\t";
                    for (int j = 0; j < results[i].size(); ++j) {
                        file << std::to_string(results[i][j]) << " ";  // Store each result in the JSON array
                    }
                    file << std::endl;
                }
                file.close();
            } else {
                std::cout << "Unable to open file" << std::endl;
            }
        }
    }
    return 0;
}