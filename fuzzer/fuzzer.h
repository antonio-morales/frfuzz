/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>

#include "global.h"
#include "utils/filesys.h"
#include "utils/process.h"
#include "utils/utils.h"
#include "utils/x11.h"

#include "fuzzerPool.h"

class fuzzer : public process {

  public:
    fuzzer();

    ~fuzzer();

    inline uint32_t get_id() const { return id; }

    inline std::string get_name() const { return name; }

    inline std::string get_target() const { return target_path; }

    /*
    std::string read_property(std::string ){
    }
    */

    virtual uint32_t get_crashes() = 0;

    virtual bool run() = 0;

  protected:
    std::string name;

    std::string target_path;

    std::string cmdline;

  private:
    uint32_t id;

    friend class fuzzerPool;
};

void fuzzer_kill();

void inputs_gather(std::filesystem::path parent_folder, std::string output_folder, std::filesystem::path output_path,
                   std::vector<std::filesystem::path> input_folders);
