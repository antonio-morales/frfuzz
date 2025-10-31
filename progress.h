/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <stdint.h>

#include <functional>
#include <iostream>
#include <shared_mutex>
#include <source_location>
#include <string>
#include <thread>

#include "utils/utils.h"

namespace progress {

struct status {
    float progress;
    std::string currentItem;
};

extern std::unordered_map<std::size_t, status> progressMap;

extern std::shared_mutex mutex;

void start(const std::source_location location = std::source_location::current());

// Update for threads
void update(uint32_t curr, uint32_t total, std::string currItem = "");

status get(uint64_t id);

} // namespace progress
