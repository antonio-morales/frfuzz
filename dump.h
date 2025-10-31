/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <filesystem>
#include <regex>
#include <unordered_set>

#include "error.h"
#include "github/API.h"
#include "html/parser.h"
#include "network/HTTP.h"
#include "output.h"
#include "progress.h"

#include <nlohmann/json.hpp>

void ossFuzz_dump(std::string dir, std::set<std::string> projects = {}, std::set<std::string> dates = {});

bool dump(std::string URL, std::string outputDir);

void myDump(int epoch);