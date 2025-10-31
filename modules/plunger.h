/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>

#include "coverage/coverage.h"
#include "coverage/lcov.h"
#include "crypto/secrets.h"
#include "json.h"
#include "llm/llm.h"
#include "network/HTTP.h"
#include "validator/yara.h"

int plunger(FRglobal &ctx, std::filesystem::path output_folder, std::filesystem::path yara_rules_file, uint32_t interval);
