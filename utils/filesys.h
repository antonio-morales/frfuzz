/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <magic.h>
#include <map>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "error.h"
#include "utils.h"

struct file_t {
    std::string path;
    std::string content;
};

class folder_snapshot {

  private:
    std::unordered_map<std::string, std::array<unsigned char, 20>> file_hash;

  public:
    folder_snapshot(std::filesystem::path folder_path);

    std::vector<std::filesystem::path> diff(const std::filesystem::path &folder_path, bool recursive = true);

    // std::vector<std::string> diff(const folder_snapshot &snapshot);
};

std::string get_mime_info(const std::filesystem::path &path, std::string info = "");

bool is_executable_file(const std::filesystem::path &path);

char *read_file(const std::string path, size_t &size);

char *read_fd(int fd, size_t &size);
std::string read_fd(int fd);

std::string read_file(const std::filesystem::path file_path);
std::string read_file(const std::filesystem::path file_path, int &error_code);

std::vector<uint8_t> read_file_bytes(std::filesystem::path file_path);
std::vector<uint8_t> read_file_bytes(std::filesystem::path file_path, std::error_code &error);

bool write_file(const std::string path, std::string file_contents);

bool create_dir(std::filesystem::path path, bool recursive = false);

std::vector<std::filesystem::path> files_to_disk(std::vector<file_t> files, std::string local_folder, bool auto_numbering = false);

std::string get_hostname();

std::filesystem::path get_home_dir();

std::time_t last_modified(std::filesystem::path path);

std::filesystem::path unique_folder_name(const std::filesystem::path &parent_folder, std::string folder_prefix, std::string type);

bool copy_files(const std::filesystem::path &input_folder, const std::filesystem::path &output_folder, std::string extension, size_t max_length,
                bool reverse);
