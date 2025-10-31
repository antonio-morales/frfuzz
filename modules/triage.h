/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "fuzzer/engines/afl.h"
#include "global.h"
#include "utils/process.h"
#include "utils/utils.h"

enum class BUG_TYPE {
    HEAP_BUFFER_OVERFLOW,        //" heap-buffer-overflow "
    HEAP_BUFFER_OVERFLOW_READ,   //" heap-buffer-overflow READ " TODO: Show those with the greatest size
    HEAP_BUFFER_OVERFLOW_WRITE,  //" heap-buffer-overflow WRITE "
    STACK_BUFFER_OVERFLOW,       //" stack-buffer-overflow "
    STACK_BUFFER_OVERFLOW_READ,  //" stack-buffer-overflow READ "
    STACK_BUFFER_OVERFLOW_WRITE, //" stack-buffer-overflow WRITE "
    SEGV,                        //" SEGV "
    SEGV_READ,                   //" SEGV READ "
    SEGV_WRITE,                  //" SEGV WRITE "
    ALLOCATION_OVERFLOW,         //" requested allocation size "
    FPE,                         //" FPE "
    OOM,                         //" OOM "
    UAF,                         //" use-after-free "
    UAF_READ,                    //" use-after-free READ "
    UAF_WRITE,                   //" use-after-free WRITE "
    GLOBAL_BUFFER_OVERFLOW,      //" global-buffer-overflow "
    STACK_OVERFLOW,              //" stack-overflow "
    NEGATIVE_SIZE,               //" negative-size-param "
    INVALID_FREE,                //" attempting free on address which was not malloc "
    UNKNOWN                      //" unknown crash"
};

enum class SANITIZER {
    ASAN,   //"AddressSanitizer"
    MSAN,   //"MemorySanitizer"
    UBSAN,  //"UndefinedBehaviorSanitizer"
    TSAN,   //"ThreadSanitizer"
    CFISAN, //"Control Flow Integrity Sanitizer"
    NONE    //"None"
};

struct FR_BUG {
    // Hashed fields
    SANITIZER sanitizer = SANITIZER::NONE;
    BUG_TYPE type = BUG_TYPE::UNKNOWN;
    std::string function = "";
    std::filesystem::path file = "";
    std::size_t line = 0;

    bool operator==(const FR_BUG &) const = default; // since C++20

    // Before C++20.
    /*
    bool operator==(const FR_BUG &other) const {
        return (sanitizer == other.sanitizer && type == other.type && function == other.function && file == other.file && line == other.line);
    }
    */
};

// Custom specialization of std::hash can be injected in namespace std.
template <> struct std::hash<FR_BUG> {
    std::size_t operator()(const FR_BUG &s) const noexcept {
        std::size_t h1 = std::hash<SANITIZER>{}(s.sanitizer);
        std::size_t h2 = std::hash<BUG_TYPE>{}(s.type);
        std::size_t h3 = std::hash<std::string>{}(s.function);
        std::size_t h4 = std::hash<std::filesystem::path>{}(s.file);
        std::size_t h5 = std::hash<std::size_t>{}(s.line);

        // return h1 ^ (h2 << 1); // or use boost::hash_combine
        return h1 ^ (h2 << 1) ^ (h3 << 1) ^ (h4 << 1) ^ (h5 << 1);
    }
};

const size_t MAX_STACK_DEPTH = 8;

struct FR_NOSYM_BUG {
    SANITIZER sanitizer = SANITIZER::NONE;
    BUG_TYPE type = BUG_TYPE::UNKNOWN;

    // file, address
    std::tuple<std::filesystem::path, uint64_t> stack_trace[MAX_STACK_DEPTH];

    // std::array<std::string, MAX_STACK_DEPTH> stack_trace;

    // std::filesystem::path file = "";
    // uint64_t address;

    bool operator==(const FR_NOSYM_BUG &) const = default; // since C++20
};

template <> struct std::hash<FR_NOSYM_BUG> {
    std::size_t operator()(const FR_NOSYM_BUG &s) const noexcept {
        std::size_t h1 = std::hash<SANITIZER>{}(s.sanitizer);
        std::size_t h2 = std::hash<BUG_TYPE>{}(s.type);

        std::string stack_trace_str = "";

        for (size_t i = 0; i < MAX_STACK_DEPTH; i++) {
            stack_trace_str += std::get<0>(s.stack_trace[i]).string() + ":";
            stack_trace_str += std::to_string(std::get<1>(s.stack_trace[i])) + "__";
        }

        std::size_t h3 = std::hash<std::filesystem::path>{}(stack_trace_str);

        // return h1 ^ (h2 << 1); // or use boost::hash_combine
        return h1 ^ (h2 << 1) ^ (h3 << 1);
    }
};

struct GDB_BUG {
    std::string gdb_func;
    std::string gdb_arg;
    std::string gdb_file;
    std::size_t gdb_line;

    bool operator==(const GDB_BUG &) const = default; // since C++20
};

template <> struct std::hash<GDB_BUG> {
    std::size_t operator()(const GDB_BUG &s) const noexcept {

        std::size_t h1 = std::hash<std::string>{}(s.gdb_func);

        std::size_t h2 = std::hash<std::string>{}(s.gdb_arg);

        std::size_t h3 = std::hash<std::string>{}(s.gdb_file);

        std::size_t h4 = std::hash<std::size_t>{}(s.gdb_line);

        // return h1 ^ (h2 << 1); // or use boost::hash_combine
        return h1 ^ (h2 << 1) ^ (h3 << 1) ^ (h4 << 1);
    }
};

struct FR_CRASH {
    std::filesystem::path crash_path;
    uint64_t oob_bytes = 0;
    std::string description = "";
    std::string malloc_msg;
};

struct TRIAGE_ASAN_RESULT {
    std::unordered_map<FR_NOSYM_BUG, std::vector<FR_CRASH>> bugs;
    std::unordered_map<FR_BUG, std::vector<FR_CRASH>> sym_bugs;
    std::vector<FR_CRASH> aborted;
    std::vector<FR_CRASH> unknown;
};

struct TRIAGE_GDB_RESULT {
    std::unordered_map<GDB_BUG, std::vector<FR_CRASH>> bugs;
};

struct TRIAGE_MALLOC_RESULT {
    std::vector<FR_CRASH> detected;
};

struct TRIAGE_RESULT {

    std::string parser = "";

    TRIAGE_ASAN_RESULT *triage_asan_result = nullptr;
    TRIAGE_GDB_RESULT *triage_gdb_result = nullptr;
    TRIAGE_MALLOC_RESULT *triage_malloc_result = nullptr;
};

/*
template <>
struct std::hash<FR_BUG>
{
  std::size_t operator()(const FR_BUG& k) const
  {
    using std::size_t;
    using std::hash;
    using std::string;

    // Compute individual hash values for first,
    // second and third and combine them using XOR
    // and bit shifting:

    return ((hash<SANITIZER>()(k.sanitizer)
            ^ (hash<BUG_TYPE>()(k.type) << 1)) >> 1)
            ^ (hash<string>()(k.function) << 1)
            ^ (hash<string>()(k.file) << 1)
            ^ (hash<size_t>()(k.line) << 1);
  }
};
*/

std::optional<std::tuple<FR_NOSYM_BUG, FR_CRASH>> parse_sanitizer_output_NOSYM(std::string output, const std::filesystem::path triage_folder,
                                                                               const std::filesystem::path binary_folder);

std::optional<std::tuple<FR_BUG, FR_CRASH>> parse_sanitizer_output(std::string output, const std::filesystem::path triage_folder,
                                                                   const std::filesystem::path binary_folder);

void merge_triage_results(TRIAGE_RESULT &merged_results, const std::vector<TRIAGE_RESULT> &triage_results, std::string parser);

void triage_thread(size_t thread_id, const std::vector<std::filesystem::path> &crashes, std::string cmd_split1, std::string cmd_split2,
                   const std::filesystem::path triage_folder, const std::filesystem::path binary_folder, TRIAGE_RESULT &results, size_t repeat,
                   std::string parser_str);

std::string triage_summary(TRIAGE_RESULT &results, const std::vector<std::filesystem::path> &crashes_folders, size_t total_crashes,
                           std::string parser);

void symbolize_results(TRIAGE_RESULT &results, std::filesystem::path triage_folder);

void triage(std::string parser, std::vector<std::filesystem::path> crashes_folders, size_t repeat, const FRglobal &ctx);
