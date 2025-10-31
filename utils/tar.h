/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <libtar.h>

#include <iostream>
#include <string>

#include <filesystem>

#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>

#include "utils/process.h"

void tar_dir(std::string dir_path, std::string output_tar);

void untar(std::string input_tar, std::string output_path);