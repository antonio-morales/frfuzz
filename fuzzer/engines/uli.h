/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "utils/process.h"

int launch_ULI(uint32_t num_processes, std::string input_folder, std::string output_folder, std::string coverage_mode, size_t max_length,
               size_t timeout, std::vector<std::string> dictionary_paths, std::vector<std::string> mutators_to_load);