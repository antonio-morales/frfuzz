/* SPDX-License-Identifier: AGPL-3.0-only */
#include <filesystem>
#include <iostream>
#include <string>

#include <pwd.h>
#include <sys/stat.h>

#include "interface/updater.h"

/*
std::string run(std::string command) {

    command += " 2>&1";

    char buffer[128];
    std::string result;

    // std::cout << "Opening reading pipe" << std::endl;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Couldn't start command." << std::endl;
        return "";
    }

    while (fgets(buffer, 128, pipe) != NULL) {
        // std::cout << "Reading..." << std::endl;
        result += buffer;
    }

    auto returnCode = pclose(pipe);

    // std::cout << result << std::endl;
    // std::cout << returnCode << std::endl;

    return result;
}


std::time_t last_modified(std::filesystem::path path) {

    struct stat attr;
    stat(path.c_str(), &attr);
    return attr.st_mtime;
}
*/

int main(int argc, char *argv[]) {

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <orig_file> <dst_file>" << std::endl;
        return 1;
    }

    const std::filesystem::path orig_file = argv[1];
    const std::filesystem::path dst_file = argv[2];

    std::cout << "Local file is outdated. Updating it...";

    std::filesystem::copy(orig_file, dst_file, std::filesystem::copy_options::update_existing);

    std::filesystem::permissions(dst_file, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

    std::cout << " DONE!" << std::endl << std::endl;

    std::cout << "Program updated successfull. Please, run it again." << std::endl;

    // std::cout << std::endl << "---------------------------------" << std::endl << std::endl;
}