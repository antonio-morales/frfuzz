/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>

#include "docker.h"
#include "github/API.h"
#include "global.h"
#include "utils/filesys.h"

// Forward declaration to break circular dependency
class FRglobal;

class Campaign {

  public:
    Campaign();

    Campaign(std::string newName); // In-Memory project

    Campaign(std::string newName, std::filesystem::path store_path); // On-Disk project

    // Project attached to a database. All the elements linked to this project will be stored in the database
    Campaign(std::string newName, void *db_handler);

    ~Campaign();

    enum type_enum { NATIVE, DOCKER };

    enum stage { CREATED, IMPORTING, IMPORTED, COMPILING, READY };

    // bool init(std::filesystem::path init_folder, std::filesystem::path binary_rel_path, std::string binary_args);

    bool init(FRglobal &ctx, std::string project_name, std::string project_version);

    // bool init(const FRglobal &ctx, std::filesystem::path campaign_path, std::string project_name, std::filesystem::path binary_rel_path,
    // std::string binary_args);

    static bool is_valid_name(std::string name);

    bool save_to_disk(std::filesystem::path store_path); // Save In-Memory campaign to disk

    bool load_from_disk(std::filesystem::path campaign_path);

    bool import(std::string source, std::string parameters);

    bool target_compile(std::string build_name);

    std::filesystem::path unique_run_folder(std::string engine);

    std::string project_name() { return project_name_; }

    std::string project_version() { return project_version_; }

    // TODO: Change all "_folder" to "_path"
    std::filesystem::path campaign_path; // Current fuzzing campaign path

    std::filesystem::path src_folder;

    std::filesystem::path frfuzz_file; // Current project .frfuzz file path

    std::filesystem::path build_sh; // Path to the build.sh script

    std::filesystem::path binary_rel_path;

    std::string binary_args;

  private:
    uint32_t id;

    std::string name_;

    std::string project_name_;

    std::string project_version_;

    const std::string SRC_FOLDER = "src";
    const std::string BUILDS_FOLDER = "builds";
    const std::string FUZZERS_FOLDER = "fuzzers";
    const std::string TARGET_FOLDER = "target";

    type_enum type = NATIVE;

    stage current_stage = CREATED;

    std::filesystem::path src_path; //"src"

    std::filesystem::path builds_root_path;

    std::filesystem::path fuzzers_root_path;

    std::filesystem::path target_path;

    // std::vector<std::filesystem::path>

    std::unordered_set<std::string> build_names;

    std::vector<std::filesystem::path> executables;

    std::string make_dirs();

    bool import_ossfuzz(std::string project_name);

    bool import_github(std::string repo_url);

    bool native_compilation(std::string build_name);

    bool CMake(std::filesystem::path build_path);

    void dockers_func();
};

std::filesystem::path unique_project_folder(const std::filesystem::path &parent_folder, std::string engine);