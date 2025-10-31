/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include "../fuzzer.h"

#include "AFL_PROFILES.h"

const std::vector<std::string> afl_builds = {"__AFL",         "__AFL_ASAN",   "__AFL_UBSAN",  "__AFL_CFISAN",    "__AFL_CMPLOG",
                                             "__AFL_COMPCOV", "__AFL_CTX",    "__AFL_CALLER", "__AFL_NGRAM",     "__COV",
                                             "__ASAN",        "__ASAN_NOOPT", "__UBSAN",      "__AFL_NGRAM_ASAN"};

enum class AFL_PARALLELISM { NONE, MASTER, SLAVE };

enum class AFL_DETERMINISM { HAVOC, DETERMINISTIC };

enum class AFL_INSTRUMENTATION {
    LTO,
    LTO_ASAN,
    LTO_UBSAN,
    LTO_CFISAN,
    LTO_CMPLOG,
    LTO_COMPCOV,
    LLVM,
    LLVM_ASAN,
    LLVM_UBSAN,
    LLVM_CFISAN,
    LLVM_CMPLOG,
    LLVM_COMPCOV,
    LLVM_CTX,
    LLVM_CALLER,
    LLVM_NGRAM,
    LLVM_NGRAM_ASAN
};

enum class AFL_STRATEGY { NONE, EXPLORE, EXPLOIT };

enum class AFL_POWER_SCHEDULE {
    DEFAULT, // FAST
    EXPLORE,
    EXPLOIT,
    SEEK,
    RARE,
    MMOPT,
    LIN,
    COE,
    QUAD
};

enum class AFL_TRIM { DEFAULT, DISABLE };

enum class AFL_QUEUE_SELECTION { DEFAULT, SEQUENTIAL };

struct AFL_INSTANCE_CONFIG {
    AFL_PARALLELISM parallelism;
    AFL_DETERMINISM determinism;
    AFL_INSTRUMENTATION instrumentation;
    AFL_STRATEGY strategy;
    AFL_POWER_SCHEDULE power_schedule;
    AFL_TRIM trim;
    AFL_QUEUE_SELECTION queue_selection;
};

void fuzz_afl(std::string profileFile, size_t cores, std::string input_path, std::filesystem::path output_path, size_t max_length, size_t timeout,
              size_t memory_limit, std::string extension, std::vector<std::string> dictionary_paths, size_t cache_size, const FRglobal &ctx);

std::vector<std::filesystem::path> AFL_get_crashes(const std::filesystem::path AFL_folder);

class afl : public fuzzer {

  public:
    afl();

    afl(std::string input_dir, std::string output_dir, std::string target, std::string args);

    ~afl();

    uint32_t get_crashes();

    bool build() {

        std::string command = "afl-gcc " + target_path + " -o " + target_path + ".afl";

        std::cout << command << std::endl;

        char *argv[] = {(char *)"/bin/sh", (char *)"-c", (char *)command.c_str(), NULL};
    }

    bool run() {

        std::string command = "afl-fuzz -i " + input_directory + " -o " + output_directory + " " + target_path + " " + cmdline;

        std::cout << command << std::endl;

        char *argv[] = {(char *)"/bin/sh", (char *)"-c", (char *)command.c_str(), NULL};

        this->spawn();

        return true;
    }

  private:
    std::string input_directory = "";
    std::string output_directory = "";

    std::unordered_map<std::string, std::string> stats;

    void parse_fuzzer_stats(std::string filecontent);

    void load_stats();

    std::string get_stat(std::string stat);
};