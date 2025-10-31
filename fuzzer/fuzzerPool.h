/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include "fuzzer.h"

// Forward declaration
class fuzzer;

// fuzzer method forward declaration

class fuzzerPool {
  private:
    inline static std::unordered_map<uint32_t, fuzzer *> pool;

    inline static uint32_t id_counter = 0;

    static uint32_t new_id();

  public:
    static void add(fuzzer *f);

    static bool remove(uint32_t id);

    static fuzzer *get(uint32_t id);

    static std::vector<fuzzer *> data();
};