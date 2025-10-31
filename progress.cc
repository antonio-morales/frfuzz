/* SPDX-License-Identifier: AGPL-3.0-only */
#include "progress.h"

namespace progress {

std::unordered_map<std::size_t, status> progressMap;

std::shared_mutex mutex;

void start(const std::source_location location) {

    std::string str = location.file_name();
    str += location.function_name();

    std::size_t id = std::hash<std::string>{}(str);

    status s;
    s.progress = 0.0;
    s.currentItem = "";

    progressMap[id] = s;

    // std::cout << id << std::endl;
}

void update(uint32_t curr, uint32_t total, std::string currItem) {

    uint64_t key = toint(std::this_thread::get_id());

    status s;

    float c = curr;
    float t = total;

    s.progress = c / t;

    s.currentItem = currItem;

    // std::cout << c/t << std::endl;

    mutex.lock();

    progressMap[key] = s;

    mutex.unlock();

    // std::cout << "esto es" << std::endl;
}

status get(uint64_t id) {

    status s;
    s.progress = 0.0;
    s.currentItem = "";

    // auto findit;

    std::unordered_map<std::size_t, status>::iterator it;

    mutex.lock_shared();

    it = progressMap.find(id);

    mutex.unlock_shared();

    if (it != progressMap.end()) {
        s = it->second;
    }

    return s;
}

} // namespace progress
