/* SPDX-License-Identifier: AGPL-3.0-only */
#include "modules/autoconfig.h"

namespace autoconfig {

void list() {

    // List all the pre-configured campaigns

    std::string owner = ghapi::get_owner(CONFIGS_REPO);

    std::string repo = ghapi::get_repo(CONFIGS_REPO);

    JSON j = ghapi::get_repo_content(owner, repo, "");

    if (is_debug_enabled()) {
        std::cerr << j.dump(4) << std::endl;
    }

    if (j.contains("Error")) {
        std::cerr << "Error: " << j["Error"] << std::endl;
        exit(EXIT_FAILURE);
    }

    bool found_configs = false;

    for (auto &item : j) {
        if (item["type"] == "dir" && item["name"] == "configs") {
            found_configs = true;
        }
    }

    if (!found_configs) {
        std::cerr << "Error: Could not find 'configs' folder in the repository" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Now get all the folders in the "configs" folder
    j = ghapi::get_repo_content(owner, repo, "configs");

    // Let's gonna store it in a unordered map
    std::unordered_map<std::string, std::vector<std::string>> configs;

    for (auto &item : j) {
        if (item["type"] == "dir") {
            std::string project_name = item["name"];
            configs[project_name] = std::vector<std::string>();
        }
    }

    // Now, for each project, get the versions
    for (auto &project : configs) {
        j = ghapi::get_repo_content(owner, repo, "configs/" + project.first);
        for (auto &item : j) {
            if (item["type"] == "dir") {
                std::string version = item["name"];
                configs[project.first].push_back(version);
            }
        }
    }

    std::cout << std::endl;
    std::cout << "Available configurations:" << std::endl;
    std::cout << "-------------------------" << std::endl;
    std::cout << std::endl;
    for (auto &project : configs) {
        for (auto &version : project.second) {
            std::cout << project.first << "@" << version << std::endl;
        }
    }

    std::cout << std::endl;
}

bool install(FRglobal &ctx, std::string configuration) {

    // The configuration must be in the form PROJECT@VERSION
    if (configuration.find("@") == std::string::npos) {
        std::cerr << "Error: Invalid configuration format. It must be in the form PROJECT@VERSION" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string project_name = configuration.substr(0, configuration.find("@"));
    std::string version = configuration.substr(configuration.find("@") + 1);

    // Check if the project and version exist in the repository
    std::string owner = ghapi::get_owner(CONFIGS_REPO);
    std::string repo = ghapi::get_repo(CONFIGS_REPO);

    JSON j = ghapi::get_repo_content(owner, repo, "configs/" + project_name + "/" + version);
    if (j.is_null()) {
        std::cerr << "Error: Configuration " << configuration << " not found in the repository" << std::endl;
        exit(EXIT_FAILURE);
    }

    Campaign *campaign = new Campaign();

    campaign->init(ctx, project_name, version);

    std::string tmp_link = "";

    // Search the file "build.sh" in the folder. This is a script that will be executed to build the project
    for (auto &item : j) {
        if (item["type"] == "file" && item["name"] == "build.sh") {
            tmp_link = item["download_url"];
            break;
        }
    }

    if (tmp_link.empty()) {
        std::cerr << "Error: Did not find build.sh file in the configuration" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "➜ Downloading build.sh from " << tmp_link << std::endl;

    http::resp_t r = http::get(tmp_link, {}, "", false);

    std::string file_content = r.body;

    // std::filesystem::path tmp_path = "/tmp/" + project_name + "_" + version + "_build_file";

    write_file(campaign->build_sh, file_content);

    // Add execution permissions to the file
    std::filesystem::permissions(campaign->build_sh,
                                 std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
                                 std::filesystem::perm_options::add);

    tmp_link.clear();

    // Search the file ".frfuzz" in the folder
    for (auto &item : j) {
        if (item["type"] == "file" && item["name"] == ".frfuzz") {
            tmp_link = item["download_url"];
            break;
        }
    }

    if (tmp_link.empty()) {
        std::cerr << "Error: Did not find .frfuzz file in the configuration" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "➜ Downloading .frfuzz from " << tmp_link << std::endl;

    r = http::get(tmp_link, {}, "", false);

    file_content = r.body;

    write_file(campaign->frfuzz_file, file_content);

    // Search the file "download.link" in the folder. This is a text file that contains the download link for the project

    tmp_link.clear();

    for (auto &item : j) {
        if (item["type"] == "file" && item["name"] == "download.link") {
            // Get the raw version of the file
            tmp_link = item["download_url"];
            break;
        }
    }

    if (tmp_link.empty()) {
        std::cerr << "Error: Could not find download.link file in the configuration" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Download the file
    // std::cout << "Downloading link file from " << tmp_link << " ..." << std::endl;

    r = http::get(tmp_link, {}, "", false);

    std::string download_link = r.body;

    // Remove new line characters from download_link
    download_link.erase(std::remove(download_link.begin(), download_link.end(), '\n'), download_link.end());

    // Check that download_link is a GitHub URL
    if (ghapi::is_github_URL(download_link)) {
        std::cout << "➜ Downloading project from " << download_link << " ..." << std::endl;
    } else {
        std::cerr << "Error: Invalid download link in download.link file" << std::endl;
        // exit(EXIT_FAILURE);
    }

    std::vector<std::string> headers = {"Accept: application/vnd.github.v3.raw"};

    r = http::get(download_link, headers, "", false);

    file_content = r.body;

    std::filesystem::path tmp_path = "/tmp/" + project_name + "_" + version + "_src_file";

    write_file(tmp_path, file_content);

    folder_snapshot snapshot(campaign->campaign_path);

    std::cout << "➜ Extracting file " << download_link.substr(download_link.find_last_of('/') + 1) << " ..." << std::endl;
    untar(tmp_path, campaign->campaign_path);

    std::vector<std::filesystem::path> generated_files = snapshot.diff(campaign->campaign_path, false);

    if (generated_files.size() == 0) {
        std::cerr << "Error: No files were extracted from the downloaded file" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (generated_files.size() > 1) {
        std::cout << "Warning: More than one file was extracted from the downloaded file. The first one will be used as the source folder"
                  << std::endl;
    }

    std::string extracted_folder = generated_files[0];

    // Change the name of the extracted folder to "src"
    std::filesystem::rename(extracted_folder, campaign->src_folder);

    std::string cmd = "cd " + ctx.campaign->campaign_path.string() + " && ./build.sh";

    // launch_terminal(85, 27, 100, 100, cmd.c_str(), "");

    std::cout << "➜ Building " << project_name << "@" << version << " : " << std::endl;

    std::string out = run(cmd);
    debug() << out << std::endl;

    return true;
}

} // namespace autoconfig
