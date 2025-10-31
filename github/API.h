/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <string>

#include <iostream>

// Output files
#include <fstream>

#include <unordered_map>

#include <nlohmann/json.hpp>

#include "network/HTTP.h"

#include "utils/filesys.h"

namespace ghapi {

std::map<std::string, std::string> download_files_from_folder(std::string owner, std::string repo, std::string path = "");

file_t download_repo(std::string owner, std::string repo, std::string archive_format = "tar", std::string ref = "master");

nlohmann::json search(std::string type, std::string query, uint32_t numResults);

std::string get_owner(std::string repo_url);

std::string get_repo(std::string repo_url);

std::string get_string_Parameter(std::string owner, std::string repo, std::string parameter);

int get_int_Parameter(std::string owner, std::string repo, std::string parameter);

int getNumberStars(std::string repo_url);

int getNumberForks(std::string repo_url);

nlohmann::json get_repo_content(std::string owner, std::string repo, std::string path = "");

bool is_github_URL(std::string url);

} // namespace ghapi