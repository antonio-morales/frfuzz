/* SPDX-License-Identifier: AGPL-3.0-only */
#include "dump.h"

void copyToDisk(std::string baseURL) {

    std::ofstream myfile;
    myfile.open("example.txt");
    myfile << "Writing this to a file.\n";
    myfile.close();
}

bool mkdir(std::string path) {

    // Check if directory does not exist
    if (!std::filesystem::is_directory(path)) {

        if (!std::filesystem::create_directory(path)) {
            return false;
        }
    }

    return true;
}

bool mkdir_recursively(std::string path) {

    // Check if directory does not exist
    if (!std::filesystem::is_directory(path)) {

        if (!std::filesystem::create_directories(path)) {
            return false;
        }
    }

    return true;
}

void myDump(int epoch) {

    // Convert epoch to year, month, day
    time_t epoch_time = epoch;
    struct tm *tm_p = localtime(&epoch_time);

    char date[9];
    strftime(date, 9, "%Y%m%d", tm_p);

    nlohmann::json j = ghapi::get_repo_content("google", "oss-fuzz", "projects");

    std::set<std::string> projects;

    for (auto &iter : j.items()) {

        auto item = iter.value();

        // projectName = noQuotes(item["name"]);
        std::string projectName = item["name"];

        projects.insert(projectName);

        std::cout << projectName << std::endl;
    }

    ossFuzz_dump("/home/antonio", projects, {date});
}

void ossFuzz_dump(std::string dir, std::set<std::string> projects, std::set<std::string> dates) {

    std::string initialURL = "";

    std::string basePath = dir + "/oss-fuzz-coverage/";
    if (!mkdir(basePath)) {
        criticalError("Failed to create directory");
    }

    static uint32_t total_iter = dates.size() * projects.size();
    uint32_t curr_iter = 0;

    std::vector<std::tuple<std::string, std::string, std::string>> URL_list;

    bool already = true;

    for (auto d : dates) {
        for (auto p : projects) {

            // progress::update(curr_iter, total_iter, p);

            if (p == "binutils") {
                already = false;
            }

            if (already) {
                continue;
            }

            std::string project = p;
            std::string date = d;

            std::string outputDir = basePath + project + "/reports/" + date + "/linux";

            std::string URL = "https://storage.googleapis.com/oss-fuzz-coverage/" + project + "/reports/" + date + "/linux/report.html";

            URL_list.push_back(std::make_tuple(URL, outputDir, project));

            // dump(URL, outputDir);

            // curr_iter++;
        }
    }

    for (auto [u, o, p] : URL_list) {

        progress::update(curr_iter, total_iter, p);

        std::cout << u << std::endl;

        dump(u, o);

        curr_iter++;
    }
}

uint32_t getDepth(std::string URL) { return std::count(URL.begin(), URL.end(), '/') - 2; }

std::string getLocalPath(std::string initialURL, std::string URL, std::string outputDir) {

    int32_t diff = getDepth(initialURL) - getDepth(URL);

    // Remove 2

    if (diff > 0) {

        uint32_t removed = 0;
        // Traverse outputDir in reverse order
        std::string::reverse_iterator rit;
        for (rit = outputDir.rbegin(); rit != outputDir.rend(); ++rit) {
            if (*rit == '/') {
                removed++;
            }
            if (removed == diff) {
                outputDir.erase(rit.base() - 1, outputDir.end());
                break;
            }
        }

    } else if (diff < 0) {

        uint32_t i, start, end;

        for (i = URL.size(); i > 0; --i) {
            if (URL[i] == '/') {
                end = i;
                --i;
                break;
            }
        }

        for (; i != 0; --i) {
            if (URL[i] == '/') {
                diff++;
            }
            if (diff == 0) {
                start = i;
                outputDir += URL.substr(start, end - start);
                break;
            }
        }
    }

    return outputDir;
}

bool dump(std::string initialURL, std::string outputDir) {

    if (!mkdir_recursively(outputDir)) {
        criticalError("Failed to create directories");
    }

    std::string baseURL = std::filesystem::path(initialURL).remove_filename();

    std::vector<std::string> headers; // Can we pass a NULL??

    std::vector<std::string> pendingURLs;

    std::unordered_set<std::string> discovered;

    pendingURLs.push_back(initialURL);

    for (int i = 0; i < pendingURLs.size(); i++) {

        std::string URL = pendingURLs[i];

        std::cout << "get " << URL << std::endl;

        http::resp_t response = http::get(URL, headers);

        // std::string response = http::get(URL, headers, "http://127.0.0.1:8080");

        std::string directory = std::filesystem::path(URL).remove_filename();
        std::string filename = std::filesystem::path(URL).filename();

        std::string outPath = getLocalPath(initialURL, URL, outputDir);
        mkdir_recursively(outPath);

        std::ofstream outFile;
        outFile.open(outPath + "/" + filename);
        outFile << response.data;
        outFile.close();

        discovered.insert(URL);

        html::Parser parser(response.data, baseURL);

        std::vector<html::Hyperlink> links = parser.get_hyperlinks();

        for (auto &link : links) {

            std::string value = directory + "/" + link.value();

            std::string normalizedURL = std::filesystem::path(value).lexically_normal();

            // lexically_normal() converts https:// to https:/, so we need to fix it
            normalizedURL = std::regex_replace(normalizedURL, std::regex("https:/"), "https://");

            if (discovered.count(normalizedURL) == 0) {
                pendingURLs.push_back(normalizedURL);
                discovered.insert(normalizedURL);
            }
        }
    }

    return true;
}
