/* SPDX-License-Identifier: AGPL-3.0-only */
#include "API.h"

// TODO: Change it to a static class?
namespace ghapi {

const std::string API_URL = "https://api.github.com/";

const std::string AUTH_TOKEN = "";

std::map<std::string, std::string> download_files_from_folder(std::string owner, std::string repo, std::string path) {

    //<filename, file_contents>
    std::map<std::string, std::string> files;

    std::vector<std::string> files_url;

    nlohmann::json j = get_repo_content(owner, repo, path);

    for (auto &iter : j.items()) {

        auto item = iter.value();

        if (item.contains("download_url") && !item["download_url"].is_null()) {

            files_url.push_back(item["download_url"]);
            std::cout << files_url.back() << std::endl;

        } else if (item.contains("type") && item["type"] == "dir") {

            std::map<std::string, std::string> folder_files = download_files_from_folder(owner, repo, item["path"]);

            files.insert(folder_files.begin(), folder_files.end());
        }
    }

    for (auto &url : files_url) {

        http::resp_t response = http::get(url);

        if (response.code != 200) {
            std::cout << "Error in download_files_from_folder downloading file: " << url << std::endl;
            continue;
        }

        std::filesystem::path p(url);

        files.insert({path + "/" + p.filename().string(), response.body});
    }

    return files;
}

nlohmann::json search(std::string type, std::string query, uint32_t numResults) {

    // std::string result;

    if (type != "code" && type != "commits" && type != "issues" && type != "pulls" && type != "labels" && type != "repositories" &&
        type != "topics" && type != "users") {

        return {{"Error", "Invalid type: " + type}};
    }

    std::vector<std::string> headers;

    //"Accept: application/vnd.github.v4.raw");
    headers.push_back("Accept: application/vnd.github.text-match+json");

    if (!AUTH_TOKEN.empty()) {
        headers.push_back("Authorization: token " + AUTH_TOKEN);
    }

    uint32_t perPage = 2;

    uint32_t numRequests = (numResults % perPage) ? (numResults / perPage + 1) : numResults / perPage;

    std::string url = "https://api.github.com/search/" + type + "?q=" + query + "&per_page=" + std::to_string(perPage) + "&page=";

    nlohmann::json allItems;

    for (int i = 1; i <= numRequests; i++) {

        http::resp_t response = http::get(url + std::to_string(i), headers);

        if (response.code != 200) {
            return {{"Error", "Response code: " + std::to_string(response.code) + " in request: " + url + std::to_string(i)}};
        }

        nlohmann::json j = nlohmann::json::parse(response.body);

        if (!j.contains("items")) {
            return {{"Error", "Response does not contain items"}};
        }

        if (i == 1) {
            allItems = j;

        } else if (i < numRequests) {
            for (auto &iter : j["items"].items()) {
                allItems["items"] += iter.value();
            }

        } else {
            for (int k = 0; k < numResults % perPage; k++) {
                allItems["items"] += j["items"][k];
            }
        }
    }

    // std::cout << "items size: " << allItems["items"].size() << std::endl;

    // Open output file
    // std::ofstream output("output2.json");

    // Write to file
    // output << allItems.dump(4) << std::endl;

    // Close file
    // output.close();

    //

    // std::cout << all << std::endl;

    //+in:file+language:c+repo:llvm/llvm-project";

    // When you provide the text-match media type, you will receive an extra key in the JSON payload called text_matches that provides information
    // about the position of your search terms within the text

    return allItems;
}

int getNumberStars(std::string repo_url) {

    int numStars = get_int_Parameter(get_owner(repo_url), get_repo(repo_url), "stargazers_count");

    return numStars;
}

int getNumberForks(std::string repo_url) {

    int numForks = get_int_Parameter(get_owner(repo_url), get_repo(repo_url), "forks_count");

    return numForks;
}

std::string get_owner(std::string repo_url) {

    auto canonical = std::filesystem::weakly_canonical(repo_url);

    std::string owner = canonical.parent_path().filename();

    return owner;
}

std::string get_repo(std::string repo_url) {

    auto canonical = std::filesystem::weakly_canonical(repo_url);

    std::string repo = canonical.filename();

    return repo;
}

std::string get_string_Parameter(std::string owner, std::string repo, std::string parameter) {

    std::string url = "https://api.github.com/repos/" + owner + "/" + repo;

    http::resp_t response = http::get(url);

    if (response.code != 200) {
        return "";
    }

    nlohmann::json j = nlohmann::json::parse(response.body);

    if (!j.contains(parameter)) {
        return "";
    }

    return j[parameter];
}

int get_int_Parameter(std::string owner, std::string repo, std::string parameter) {

    std::string url = "https://api.github.com/repos/" + owner + "/" + repo;

    http::resp_t response = http::get(url);

    if (response.code != 200) {
        return -1;
    }

    nlohmann::json j = nlohmann::json::parse(response.body);

    if (!j.contains(parameter)) {
        return -1;
    }

    return j[parameter];
}

nlohmann::json get_repo_content(std::string owner, std::string repo, std::string path) {

    std::string url = API_URL + "repos/" + owner + "/" + repo + "/contents/" + path;

    std::vector<std::string> headers;

    if (!AUTH_TOKEN.empty()) {
        headers.push_back("Authorization: token " + AUTH_TOKEN);
    }

    http::resp_t response = http::get(url, headers);

    if (response.code != 200) {
        return {{"Error", "Response code: " + std::to_string(response.code) + " in request: " + url}};
    }

    nlohmann::json j = nlohmann::json::parse(response.body);

    // std::cout << j << std::endl;

    return j;
}

//<filename, file_contents>
file_t download_repo(std::string owner, std::string repo, std::string archive_format, std::string ref) {

    if (archive_format == "zip") {
        std::cout << "Error in download_repo: zip format not implemented yet" << std::endl;
        return {"", ""};

    } else if (archive_format == "tar") {

        std::string url = API_URL + "repos/" + owner + "/" + repo + "/tarball/" + ref;

        http::resp_t response = http::get(url);

        if (response.code != 200) {

            std::cout << "Error in download_repo: Api download failed; trying with direct download" << std::endl;

            // Try direct download
            url = "https://github.com/" + owner + "/" + repo + "/tarball/" + ref;

            response = http::get(url);

            if (response.code != 200) {

                std::cout << "Error in download_repo: Response code: " << response.code << " in request: " << url << std::endl;
                return {"", ""};
            }
        }

        // std::cout << response.headers << std::endl;

        // Filename by default
        std::string filename = repo + "_" + ref + ".tar.gz";

        if (response.headers.count("content-disposition") > 0) { // TODO: Warning! Case sensitive

            const std::string value = response.headers["content-disposition"];

            if (value.starts_with("attachment; filename=")) {
                filename = value.substr(21);
            }
        }

        return {filename, response.body};

    } else {
        std::cout << "Error in download_repo: Invalid archive_format: " << archive_format << std::endl;
        return {"", ""};
    }
}

bool is_github_URL(std::string url) {

    if (url.starts_with("http://github.com/") || url.starts_with("https://github.com/")) {
        return true;
    }

    return false;
}

} // namespace ghapi
