/* SPDX-License-Identifier: AGPL-3.0-only */
#include "campaign.h"

Campaign::Campaign() { name_ = "default"; }

Campaign::Campaign(std::string newName) {

    // TODO: Change by an exception
    if (!is_valid_name(newName)) {
        std::cout << "Error in Campaign::Campaign: Invalid campaign name" << std::endl;
        exit(1);
    }

    name_ = newName;
}

Campaign::Campaign(std::string newName, std::filesystem::path store_path) {

    // TODO: Change by an exception
    if (!is_valid_name(newName)) {
        std::cout << "Error in Campaign::Campaign: Invalid campaign name" << std::endl;
        exit(1);
    }

    name_ = newName;

    campaign_path = store_path / name_;

    if (!create_dir(campaign_path)) {
        // TODO: Change by an exception
        std::cout << "Error in Campaign::Campaign: Could not create campaign folder" << std::endl;
        exit(1);
    }
}

Campaign::Campaign(std::string newName, void *db_handler) {

    name_ = newName;

    // TODO: Check name in the database to avoid duplicates
}

Campaign::~Campaign() {}

bool Campaign::init(FRglobal &ctx, std::string project_name, std::string project_version) {

    ctx.campaign = this;

    this->campaign_path = ctx.PROJECTS_PATH / project_name / project_version / name_;

    // Check if the campaign folder already exists
    if (std::filesystem::exists(this->campaign_path)) {
        std::cout << "Error: Campaign folder " << this->campaign_path << " already exists" << std::endl;
        exit(EXIT_FAILURE);
    }

    create_dir(this->campaign_path, true);

    this->src_folder = this->campaign_path / "src";

    // create_dir(this->src_folder);

    this->frfuzz_file = this->campaign_path / ".frfuzz";

    this->build_sh = this->campaign_path / "build.sh";

    return true;
}

bool Campaign::is_valid_name(std::string name) {

    if (name.size() < 3 || name.size() > 64) {
        return false;
    }

    for (auto c : name) {
        if (!isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }

    return true;
}

/*
bool project::load(std::filesystem::path folder_path){

    //Restore the configuration from a configuration
    //name = ;

    //type = ;

    //current_stage = ;

    project_path = folder_path;
}
*/

bool Campaign::load_from_disk(std::filesystem::path campaign_path) {

    this->campaign_path = campaign_path;

    this->frfuzz_file = this->campaign_path / ".frfuzz";

    this->src_folder = this->campaign_path / "src";

    this->project_version_ = this->campaign_path.parent_path().filename().string();

    this->project_name_ = this->campaign_path.parent_path().parent_path().filename().string();

    // Open the .frfuzz file
    std::ifstream frfuzz_istream(this->frfuzz_file);
    if (!frfuzz_istream.is_open()) {
        std::cerr << "Error: Could not open .frfuzz file in " << this->frfuzz_file << std::endl;
        return false;
    }

    std::string campaign_name;
    if (!std::getline(frfuzz_istream, campaign_name)) {
        std::cerr << "Error: Could not read campaign name from .frfuzz file" << std::endl;
        return false;
    }
    this->name_ = campaign_name;

    std::string bin_rel_path;
    if (!std::getline(frfuzz_istream, bin_rel_path)) {
        std::cerr << "Error: Could not read binary relative path from .frfuzz file" << std::endl;
        return false;
    }
    this->binary_rel_path = bin_rel_path;

    std::string bin_args;
    if (!std::getline(frfuzz_istream, bin_args)) {
        std::cerr << "Error: Could not read binary arguments from .frfuzz file" << std::endl;
        return false;
    }
    this->binary_args = bin_args;

    frfuzz_istream.close();

    return true;
}

bool Campaign::import(std::string source, std::string parameters) {

    if (this->current_stage != CREATED) {
        std::cout << "Error: Campaign was already imported" << std::endl;
        return false;
    }

    if (source == "wizzard") {
        // Wizzard

    } else if (source == "ossfuzz") {
        if (!import_ossfuzz(parameters)) {
            std::cout << "Error in Campaign:import: Could not import campaign from oss-fuzz" << std::endl;
            return false;
        }

    } else if (source == "github") {
        if (!import_github(parameters)) {
            std::cout << "Error in Campaign:import: Could not import campaign from github" << std::endl;
            return false;
        }

    } else {
        std::cout << "Error: Unknown source" << std::endl;
        return false;
    }

    current_stage = IMPORTED;

    return true;
}

bool Campaign::target_compile(std::string build_name) {

    if (current_stage != IMPORTED) {
        std::cout << "Error: Campaign is not ready for compiling" << std::endl;
        return false;
    }

    if (type == NATIVE) {

        native_compilation(build_name);

    } else if (type == DOCKER) {

    } else {
        std::cout << "Error: Unknown campaign type" << std::endl;
        return false;
    }

    return true;
}


/////////////////////////// PRIVATE ///////////////////////////

std::string Campaign::make_dirs() {

    // UserData/Downloads/binutils

    src_path = campaign_path / SRC_FOLDER;
    builds_root_path = campaign_path / BUILDS_FOLDER;
    fuzzers_root_path = campaign_path / FUZZERS_FOLDER;
    target_path = campaign_path / TARGET_FOLDER;

    create_dir(src_path);
    create_dir(builds_root_path);
    create_dir(fuzzers_root_path);
    create_dir(target_path);
}

bool Campaign::import_ossfuzz(std::string campaign_name) {

    src_path = "src_docker";
    make_dirs();

    std::map<std::string, std::string> files = ghapi::download_files_from_folder("google", "oss-fuzz", "projects/" + campaign_name);

    // TODO: Change by files_to_disk
    for (auto f : files) {

        std::string path = src_path.string() + "/" + f.first;

        std::ofstream outfile(path, std::ofstream::binary);

        outfile.write(f.second.c_str(), f.second.size());

        outfile.close();

        // std::cout << f.first << " " << f.second << std::endl;
    }

    return true;
}

bool Campaign::import_github(std::string repo_url) {

    make_dirs();

    // Remove .git from the end of repo_url
    if (repo_url.substr(repo_url.length() - 4, 4) == ".git") {
        repo_url = repo_url.substr(0, repo_url.length() - 4);
    }

    std::string owner = ghapi::get_owner(repo_url);
    std::string repo = ghapi::get_repo(repo_url);

    file_t tar_file = ghapi::download_repo(owner, repo);

    if (!tar_file.path.ends_with(".tar.gz")) {
        std::cout << "Error in import_github: tar file does not end with .tar.gz" << std::endl;
        return false;
    }

    auto saved_files = files_to_disk({tar_file}, TMP_PATH, true);
    if (saved_files.size() != 1) {
        std::cout << "Error in import_github: Could not write tar file to disk" << std::endl;
        return false;
    }

    untar(saved_files[0], TMP_PATH);

    // Move extracted files to PROJECTS_PATH
    std::filesystem::path extracted_path = TMP_PATH + "/" + tar_file.path.substr(0, tar_file.path.length() - strlen(".tar.gz"));

    std::filesystem::copy(extracted_path, target_path, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    /*

    //download_files_from_folder is so much slower than use download_repo

    std::map<std::string,std::string> files = ghapi::download_files_from_folder(owner, repo);

    std::string downloads_path = create_dirs();

    bool result = files_to_disk(files, downloads_path);
    */

    return true;
}

bool Campaign::native_compilation(std::string build_name) {

    if (build_names.contains(build_name)) {
        std::cout << "Error in campaign::native_compilation: Build name already exists" << std::endl;
        return false;
    }

    build_names.insert(build_name);

    std::filesystem::path build_path = builds_root_path / build_name;
    create_dir(build_path);

    bool cmake;
    bool configure;
    bool makefile;

    // Check if there is a cmake file in the src folder
    if (std::filesystem::exists(target_path.string() + "/CMakeLists.txt")) {
        cmake = 1;
    }

    // Check if there is a configure file in the src folder
    if (std::filesystem::exists(target_path.string() + "/configure")) {
        configure = 1;
    }

    // Check if there is a makefile in the src folder
    if (std::filesystem::exists(target_path.string() + "/Makefile")) {
        makefile = 1;
    }

    if (cmake && !configure && !makefile) {

        CMake(build_path);
    }

    return true;
}

bool Campaign::CMake(std::filesystem::path build_path) {

    std::cout << "CMake" << std::endl;

    if (chdir(build_path.c_str()) == -1) {
        std::cout << "Error in campaign::CMake: Could not change directory to build_path" << std::endl;
        return false;
    }

    /*
    if(setenv("CXXFLAGS", "-std=c++14", 1) == -1){
        std::cout << "Error in project::CMake: Could not set environment variable" << std::endl;
        return false;
    }
    */

    // Release / Debug
    std::string BUILD_TYPE = "Debug";

    std::string command = "cmake -DCMAKE_BUILD_TYPE=" + BUILD_TYPE + " " + target_path.string();
    std::string output = run(command);
    std::cout << output << std::endl;

    folder_snapshot snapshot(build_path);

    // make
    command = "make -j8";
    std::cout << "Running make" << std::endl << std::endl;
    output = run(command);
    std::cout << output << std::endl;

    std::vector<std::filesystem::path> generated_files = snapshot.diff(build_path);

    // Look for executables files in the build folder

    // std::vector<std::filesystem::path> executables;

    for (auto &it : generated_files) {

        // std::cout << it << std::endl;

        if (is_executable_file(it)) {

            // it.relative_path(project_path);

            std::string relative_path = std::filesystem::relative(it, build_path);

            executables.push_back(relative_path);

            /*
            std::cout << it << " is executable file" << std::endl;
            std::string mime_info = get_mime_info(it);
            std::cout << it << " : " << mime_info << std::endl;
            std::cout << std::endl;
            */
        }
    }

    // Copy binaries to fuzzers folder

    exit(0);

    return true;
}

void Campaign::dockers_func() {

    // std::string PROJECT = "binutils";

    HTTP_PROXY = "127.0.0.1:8080";

    std::string folder = DOWNLOADS_PATH + "/" + name_;

    std::string response = docker::image_build(folder, name_);

    std::cout << response << std::endl;

    // Create container
    std::string container_id = docker::container_create_tty(name_);

    // std::string container_id = docker::container_create_tty("binutils");

    // Start container
    docker::container_start(container_id);

    std::string exec_id = docker::exec_create(container_id, "compile");

    response = docker::exec_start(exec_id);

    std::cout << response << std::endl;

    while (1) {

        nlohmann::json j = docker::exec_inspect(exec_id);

        if (!j.contains("Running")) {
            std::cout << "Error in: response does not contain Running" << std::endl;
        }

        std::cout << "Running = " << j["Running"] << std::endl;

        if (j["Running"] == false) {
            break;
        }

        sleep(5);
    }

    exit(0);
}

std::filesystem::path Campaign::unique_run_folder(std::string engine) {

    std::string folder_prefix = engine + "_output";

    return unique_folder_name(this->campaign_path, folder_prefix, "ExtendedDescription");
}
