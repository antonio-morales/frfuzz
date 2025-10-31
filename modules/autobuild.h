/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "fuzzer/engines/afl.h"

void autobuild(std::string build_system, std::string afl_instr, std::string compile, std::string opt_args, const FRglobal &ctx);