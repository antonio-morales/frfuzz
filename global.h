/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <string>

#include "campaign.h"
#include "db.h"
#include "docker.h"

const std::string CONFIGS_REPO = "https://github.com/antonio-morales/frfuzz_configs";

// Forward declaration to avoid circular include
class Campaign;

class FRglobal {

  public:
    FRglobal &init();

    grDB *global_db = nullptr;

    std::string CURRENT_USER;

    std::filesystem::path HOME_PATH;

    std::filesystem::path FRFUZZ_PATH;

    std::filesystem::path PROJECTS_PATH;

    std::filesystem::path GLOBAL_DB_PATH;

    size_t numThreads = 1;

    Campaign *campaign;

    uint32_t debug_level = 0;

    // Campaign{}

    /*

    std::filesystem::path src_folder;

    std::filesystem::path frfuzz_file; // Current project .frfuzz file path

    std::filesystem::path binary_rel_path;

    std::string binary_args;
    */

    //}

  private:
};

// inline std::string ROOT_PATH;

// inline std::string USERDATA_PATH;

inline std::string DOWNLOADS_PATH;

inline std::string TMP_PATH;

// Current project path
// TODO: //Currently handled with a cookie??
inline std::string CURRENT_PROJECT;

inline std::string HTTP_PROXY;
