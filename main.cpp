#include <iostream>
#include <vector>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>  // Include JSON library
#include "psi_protocols.h"

using json = nlohmann::json;  // For convenience

int main() {
    Keys keys;
    key_gen(&keys, 1024, 2, 3);

    std::vector<long> client1_set({1, 3, 5, 7});
    std::vector<long> client2_set({2, 3, 4, 5});
    std::vector<long> server_set({1, 2, 5, 6});

    std::vector<std::vector<long>> all_sets = {client1_set, client2_set, server_set};
    std::vector<std::future<std::vector<long>>> futures;

    for (int i = 0; i < all_sets.size(); ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            std::vector<std::vector<long>> client_sets;
            for (int j = 0; j < all_sets.size(); ++j) {
                if (j != i) {
                    client_sets.push_back(all_sets[j]);
                }
            }
            return threshold_multiparty_psi(client_sets, all_sets[i], 2, 16, 4, 1, keys);
        }));
    }

    std::vector<std::vector<long>> results;
    for (auto& fut : futures) {
        results.push_back(fut.get());
    }

    // Create a JSON object
    json j;
    j["results"] = json::array();  // Create a JSON array to store results

    for (int i = 0; i < results.size(); ++i) {
        j["results"].push_back(results[i]);  // Store each result in the JSON array
    }

    // Output JSON to the console
    std::cout << j.dump(4) << std::endl;  // Pretty print with a 4-space indent

    // Optionally, write JSON to a file
    std::ofstream file("output.json");
    file << j.dump(4);  // Pretty print to file
    file.close();

    return 0;
}
