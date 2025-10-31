/* SPDX-License-Identifier: AGPL-3.0-only */
#include "ossfuzz.h"

namespace ossfuzz {

std::string get_repo_url(std::string project) {

    http::resp_t response = http::get("https://raw.githubusercontent.com/google/oss-fuzz/master/projects/" + project + "/project.yaml");

    YAML yaml(response.body);

    std::string repo_url = yaml.search("main_repo");

    if (repo_url != "") {

        // Remove quotes from the beginning and end of repo_url
        if (repo_url[0] == '\'' && repo_url[repo_url.length() - 1] == '\'') {
            repo_url = repo_url.substr(1, repo_url.length() - 2);
        }

        // Remove .git from the end of repo_url
        if (repo_url.substr(repo_url.length() - 4, 4) == ".git") {
            repo_url = repo_url.substr(0, repo_url.length() - 4);
        }
    }

    return repo_url;
}

} // namespace ossfuzz
