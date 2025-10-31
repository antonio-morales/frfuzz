/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "utils/process.h"

struct coverage {
    int lines_cov;
    int lines;
    int functions_cov;
    int functions;
    int regions_cov;
    int regions;
};

void coverage(std::vector<std::filesystem::path> output_folders, const FRglobal &ctx, bool html_report = true);

void do_break(std::string breakpoint, std::vector<std::filesystem::path> output_folders, const FRglobal &ctx);
