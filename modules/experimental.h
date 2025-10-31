/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/filesys.h"

void exp_patterns(const std::vector<std::filesystem::path> &input_files, std::filesystem::path output_folder);