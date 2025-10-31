/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <dirent.h>

#include "graph/node.h"
#include "utils/process.h"

bool generate_dot(std::filesystem::path queue_folder);