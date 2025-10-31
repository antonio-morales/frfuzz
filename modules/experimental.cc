/* SPDX-License-Identifier: AGPL-3.0-only */
#include "experimental.h"

std::string patterns(const std::vector<std::filesystem::path> &input_files) {

    const size_t NUM_PRINT = 100;

    const size_t MAX_SIZE = 20;

    std::string patterns = "";

    const size_t num_files = input_files.size();

    for (size_t SIZE = 4; SIZE < MAX_SIZE; SIZE++) {

        std::cout << "SIZE: " << SIZE << std::endl;

        std::unordered_map<std::string, int> patterns_map;

        int n = 0;

        for (const auto &f : input_files) {

            if (n % 100 == 0) {
                std::cout << "Processing file " << n << " of " << num_files << std::endl;
            }

            std::string content = read_file(f);

            size_t pos = 0;

            while (SIZE <= content.size() && pos < (content.size() - SIZE)) {

                std::string word = content.substr(pos, SIZE);

                // We want to store the number of times that each pattern appears
                if (patterns_map.find(word) == patterns_map.end()) {
                    patterns_map[word] = 1;
                } else {
                    patterns_map[word]++;
                }

                pos++;
            }

            n++;
        }

        // Show the 5 elements with the highest frequency
        std::vector<std::pair<std::string, int>> patterns_vec(patterns_map.begin(), patterns_map.end());

        std::sort(patterns_vec.begin(), patterns_vec.end(),
                  [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) { return a.second > b.second; });

        for (int i = 0; i < NUM_PRINT && i < patterns_vec.size(); i++) {
            // patterns += patterns_vec[i].first + " ";
            // std::cout << patterns_vec[i].first << " -> " << patterns_vec[i].second << std::endl;

            std::cout << "\"";

            // Print the string as bytes
            for (int j = 0; j < SIZE; j++) {

                unsigned char c = (unsigned char)patterns_vec[i].first[j];

                if (std::isprint(c)) {
                    std::cout << c;
                } else {
                    std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
                }

                // std::cout << "0x" << std::hex << (int)(unsigned char)patterns_vec[i].first[j] << " ";
            }

            std::cout << "\"";

            // std::cout << "(\"" << patterns_vec[i].first << "\")";
            std::cout << " -> " << std::dec << patterns_vec[i].second << " times" << std::endl;
        }

        std::cout << "--------------------------------\n" << std::endl;
    }

    return patterns;
}

void exp_patterns(const std::vector<std::filesystem::path> &input_files, std::filesystem::path output_folder) {

    // Write an HTML summary

    std::filesystem::path summary_path = std::filesystem::path(output_folder).parent_path();
    summary_path /= "patterns.html";

    if (std::filesystem::exists(summary_path)) {
        summary_path = summary_path.string().erase(summary_path.string().size() - 5, 5);
        summary_path +=
            "_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        summary_path += ".html";
    }

    std::ofstream file;

    // file.open(summary_path);

    std::string html_summary = patterns(input_files);

    patterns(input_files);

    // file << html_summary;

    // file.close();
}
