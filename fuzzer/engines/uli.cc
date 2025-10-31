/* SPDX-License-Identifier: AGPL-3.0-only */
#include "uli.h"

int launch_ULI(uint32_t num_processes, std::string input_folder, std::string output_folder, std::string coverage_mode, size_t max_length,
               size_t timeout, std::vector<std::string> dictionary_paths, std::vector<std::string> mutators_to_load) {

    std::string cmd("ULIEngine");

    cmd += " -o " + output_folder;

    if (input_folder != "") {
        cmd += " -i " + input_folder;
    }

    if (coverage_mode != "") {
        cmd += " -c " + coverage_mode;
    }

    if (num_processes > 0) {
        cmd += " -n " + std::to_string(num_processes);
    }

    if (max_length > 0) {
        cmd += " -s " + std::to_string(max_length);
    }

    if (dictionary_paths.size() > 0) {

        for (auto dictionary_path : dictionary_paths) {

            // Check if dictionary exists
            if (!std::filesystem::exists(dictionary_path)) {
                std::cerr << "Dictionary file does not exist: " << dictionary_path << std::endl;
                exit(EXIT_FAILURE);
            }

            cmd += " -d ";
            cmd += dictionary_path;
        }
    }

    if (mutators_to_load.size() > 0) {

        for (auto mutator : mutators_to_load) {
            cmd += " -m ";
            cmd += mutator;
        }
    }

    std::string env = "";

    const int width_columns = 120;
    const int height_columns = 54;

    size_t Pos_X = 0;
    size_t Pos_Y = 0;

    int pid = launch_terminal(width_columns, height_columns, Pos_X, Pos_Y, cmd.c_str(), env);

    return pid;
}
