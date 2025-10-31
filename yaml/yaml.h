/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <sstream>
#include <string>
#include <unordered_map>

#include "utils/utils.h"

class YAML {

  private:
    std::string prvText;

    std::unordered_map<std::string, std::vector<std::string>> prvMap;

  public:
    // Default constructor
    YAML(const std::string &inputText);

    std::string search(std::string key);
};