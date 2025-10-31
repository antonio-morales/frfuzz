/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <string.h>
#include <termios.h>

#include <openssl/sha.h>

#include "filesys.h"

std::array<unsigned char, 20> git_hash_object(std::filesystem::path p);

uint32_t HexToBytes(const std::string &hex, uint8_t *outBuf);

std::string hex_decode(const std::string &hex);

bool is_hex_string(const std::string &s);

std::string random_password(uint32_t length = 10);

std::string base64_encode(const std::string &in);

std::string base64_decode(const std::string &in);

inline bool is_base64(const std::string &s) {

    std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789+/";

    for (char c : s) {
        if (base64_chars.find(c) == std::string::npos && c != '=') {
            return false;
        }
    }

    return true;
}

uint64_t toint(const std::thread::id &id);

std::string format_duration(uint64_t duration);

std::string current_time_str();

static inline void to_lower(std::string &c) { std::transform(c.begin(), c.end(), c.begin(), ::tolower); }

static inline bool is_number(const std::string &s) { return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit); }

// Case-insensitive "starts_with" C++ function
static inline bool istarts_with(std::string c, std::string s) {

    std::transform(c.begin(), c.end(), c.begin(), ::tolower);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    if (c.starts_with(s)) {
        return true;
    }

    return false;
}

// Split string into substring
static inline std::vector<std::string> split(std::string s, char delim) {

    std::vector<std::string> res;

    std::stringstream ss(s);

    std::string item;

    while (std::getline(ss, item, delim)) {
        res.push_back(item);
    }

    return res;
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

/*
// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}
*/

// trim from both ends (copying)
static inline std::string trim(std::string s) {

    rtrim(s);
    ltrim(s);

    return s;
}

std::unordered_map<std::string, std::string> parse_configuration_file(std::string file_path);

std::string memory_dump(void *ptr, size_t size);

/// Utility function to decode a ULEB128 value.
inline uint64_t decodeULEB128(const uint8_t *p, unsigned *n = nullptr, const uint8_t *end = nullptr, const char **error = nullptr) {
    const uint8_t *orig_p = p;
    uint64_t Value = 0;
    unsigned Shift = 0;
    if (error)
        *error = nullptr;
    do {
        if (end && p == end) {
            if (error)
                *error = "malformed uleb128, extends past end";
            if (n)
                *n = (unsigned)(p - orig_p);
            return 0;
        }
        uint64_t Slice = *p & 0x7f;
        if (Shift >= 64 || Slice << Shift >> Shift != Slice) {
            if (error)
                *error = "uleb128 too big for uint64";
            if (n)
                *n = (unsigned)(p - orig_p);
            return 0;
        }
        Value += uint64_t(*p & 0x7f) << Shift;
        Shift += 7;
    } while (*p++ >= 128);
    if (n)
        *n = (unsigned)(p - orig_p);

    return Value;
}

inline int64_t decodeSLEB128(const uint8_t *p, unsigned *n = nullptr, const uint8_t *end = nullptr, const char **error = nullptr) {
    const uint8_t *orig_p = p;
    int64_t Value = 0;
    unsigned Shift = 0;
    uint8_t Byte;
    if (error)
        *error = nullptr;
    do {
        if (end && p == end) {
            if (error)
                *error = "malformed sleb128, extends past end";
            if (n)
                *n = (unsigned)(p - orig_p);
            return 0;
        }
        Byte = *p++;
        Value |= (uint64_t(Byte & 0x7f) << Shift);
        Shift += 7;
    } while (Byte >= 128);
    // Sign extend negative numbers if needed.
    if (Shift < 64 && (Byte & 0x40))
        Value |= (-1ULL) << Shift;
    if (n)
        *n = (unsigned)(p - orig_p);
    return Value;
}

std::string get_password_masked(const std::string &prompt);
