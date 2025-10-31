/* SPDX-License-Identifier: AGPL-3.0-only */
#include "yaml.h"

YAML::YAML(const std::string &inputText) {

    prvText = inputText;

    std::istringstream iss(prvText);

    for (std::string line; std::getline(iss, line);) {

        if (line.find(":") != std::string::npos) {

            std::string key = trim(line.substr(0, line.find(":")));
            std::string value = trim(line.substr(line.find(":") + 1));

            prvMap[key].push_back(value);
        }
    }
}

std::string YAML::search(std::string key) {

    std::string value = "";

    std::istringstream iss(prvText);

    for (std::string line; std::getline(iss, line);) {
        if (line.find(key) != std::string::npos) {

            value = trim(line.substr(line.find(key) + key.length() + 1));
            break;
        }
    }

    return value;
}
