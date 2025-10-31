/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

struct node {
    int32_t id = -1;
    int32_t parent_id = -1;
    std::vector<int32_t> children;
    int32_t depth = -1;
    char *filename = nullptr;
};

node testcase_to_node(std::string testcase_filename);