/* SPDX-License-Identifier: AGPL-3.0-only */
#include "fuzzerPool.h"

uint32_t fuzzerPool::new_id() {

    // TODO: read this value from database, to have a historic of all fuzzers

    return fuzzerPool::id_counter++;
}

void fuzzerPool::add(fuzzer *f) {

    f->id = new_id();

    pool.insert({f->id, f});
}

bool fuzzerPool::remove(uint32_t id) {
    if (pool.find(id) != pool.end()) {
        pool.erase(id);
        return true;
    }
    return false;
}

fuzzer *fuzzerPool::get(uint32_t id) {

    if (pool.count(id) > 0) {
        return pool[id];
    }

    return nullptr;
}

std::vector<fuzzer *> fuzzerPool::data() {

    std::vector<fuzzer *> v;

    for (auto &f : pool) {
        v.push_back(f.second);
    }

    return v;
}
