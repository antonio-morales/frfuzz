/* SPDX-License-Identifier: AGPL-3.0-only */
#include "init.h"

bool check_docker_http_api() {

    auto conf = parse_configuration_file("/lib/systemd/system/docker.service");

    if (conf.find("ExecStart") != conf.end()) {

        std::string ExecStart = conf["ExecStart"];

        // Find -H tcp:// in ExecStart
        if (ExecStart.find("-H tcp://") == std::string::npos) {

            std::cout << "Docker HTTP API access is not enabled in your system." << std::endl;
            std::cout << "Run the following order to enable it: " << std::endl;
            std::cout << "sudo sed -i \"s/^ExecStart=.*\/ExecStart=\\/usr\\/bin\\/dockerd -H fd:\\/\\/ -H tcp:\\/\\/127.0.0.1:2375/g\" "
                         "/lib/systemd/system/docker.service"
                      << std::endl;
            std::cout << "sudo systemctl daemon-reload" << std::endl;
            std::cout << "sudo systemctl restart docker.service" << std::endl;
        } else {
            return true;
        }

    } else {
        std::cout << "Error parsing docker.service file" << std::endl;
    }

    return false;
}

/*
std::list<std::string> load_projects(){

    // Read all existing projects in PROJECTS_PATH
    std::list<std::string> projects;

    for (auto const &dir_entry : std::filesystem::directory_iterator(PROJECTS_PATH)) {

        std::string project_name = dir_entry.path().filename().string();

        if (project::is_valid_name(project_name)) {
            projects.push_back(project_name);
        }

        // TODO: Check other files validity (e.g. database)
    }

    return projects;
}
*/