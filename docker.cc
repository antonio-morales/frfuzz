/* SPDX-License-Identifier: AGPL-3.0-only */
#include "docker.h"

namespace docker {

bool check_docker_http_api() {

    auto conf = parse_configuration_file("/lib/systemd/system/docker.service");

    if (conf.find("ExecStart") != conf.end()) {

        std::string ExecStart = conf["ExecStart"];

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

std::string image_build(std::string folder_path, std::string image_name) {

    std::string tar_file = TMP_PATH + "/" + image_name + "_Dockerfile.tar.gz";

    tar_dir(folder_path, tar_file);

    size_t size;
    char *data = read_file(tar_file, size);

    std::string URL = BASE_URL + "/build?t=" + image_name;

    http::resp_t response = http::post(URL, data, size, {"Content-Type:application/tar"});

    if (response.code != 200) {
        std::cout << "Error in docker::build_image" << std::endl;
        exit(1);
    }

    return response.body;
}

std::string container_create(std::string image_name, JSON aditional_opts) {

    std::string URL = BASE_URL + "/containers/create";

    JSON options = {{"Image", image_name}, {"Env", {"FUZZING_LANGUAGE=C++"}}};

    options.merge_patch(aditional_opts);

    std::string data = nlohmann::to_string(options);

    http::resp_t response = http::post(URL, data.c_str(), data.length(), {"Content-Type:application/json"});

    if (response.code != 201) {
        std::cout << "Error creating container" << std::endl;
        exit(1);
    }

    JSON j = JSON::parse(response.body);

    if (!j.contains("Id")) {
        std::cout << "Error in docker::container_create: response does not contain Id" << std::endl;
    }

    std::cout << "Id = " << j["Id"] << std::endl;

    std::string container_id = j["Id"];

    return container_id;
}

std::string container_create_tty(std::string image_name) {

    JSON options = {{"OpenStdin", true},
                    {
                        "Tty",
                        true,
                    },
                    {"Entrypoint", "/bin/bash"}};

    std::string container_id = container_create(image_name, options);

    return container_id;
}

std::string container_start(std::string container_id) {

    std::string URL = BASE_URL + "/containers/" + container_id + "/start";

    std::string data = "";

    http::resp_t response = http::post(URL, data.c_str(), data.length(), {"Content-Type:application/json"});

    switch (response.code) {
    case 204:
        /* No error*/
        break;

    case 304:
        std::cout << "Error in docker::container_start: Container already started" << std::endl;
        exit(1);

    case 404:
        std::cout << "Error in docker::container_start: No such container" << std::endl;
        exit(1);

    case 500:
        std::cout << "Error in docker::container_start: Server error" << std::endl;
        exit(1);

    default:
        std::cout << "Error in docker::container_start: Unknown error" << std::endl;
        exit(1);
    }

    return response.body;
}

std::string exec_create(std::string container_id, std::string cmd) {

    std::string URL = BASE_URL + "/containers/" + container_id + "/exec";

    JSON options = {{"Cmd", {cmd}}, {"Env", {"FUZZING_LANGUAGE=C++"}}};

    std::string data = nlohmann::to_string(options);

    http::resp_t response = http::post(URL, data.c_str(), data.length(), {"Content-Type:application/json"});

    switch (response.code) {
    case 201:
        /* No error*/
        break;

    case 404:
        std::cout << "Error in docker::exec_create: No such container" << std::endl;
        exit(1);

    case 409:
        std::cout << "Error in docker::exec_create: Container already started" << std::endl;
        exit(1);

    case 500:
        std::cout << "Error in docker::exec_create: Server error" << std::endl;
        exit(1);

    default:
        std::cout << "Error in docker::exec_create: Unknown error" << std::endl;
        exit(1);
    }

    JSON j = JSON::parse(response.body);

    if (!j.contains("Id")) {
        std::cout << "Error in docker::exec_create: response does not contain Id" << std::endl;
    }

    std::cout << "Id = " << j["Id"] << std::endl;

    std::string exec_id = j["Id"];

    return exec_id;
}

std::string exec_start(std::string exec_id) {

    std::string URL = BASE_URL + "/exec/" + exec_id + "/start";

    std::string data = "{}";

    http::resp_t response = http::post(URL, data.c_str(), data.length(), {"Content-Type:application/json"});

    switch (response.code) {
    case 200:
        /* No error*/
        break;

    case 404:
        std::cout << "Error in docker::exec_start: No such exec instance" << std::endl;
        exit(1);

    case 409:
        std::cout << "Error in docker::exec_start: Container is stopped or paused" << std::endl;
        exit(1);

    default:
        std::cout << "Error in docker::exec_start: Unknown error" << std::endl;
        exit(1);
    }

    return response.body;
}

JSON exec_inspect(std::string exec_id) {

    std::string URL = BASE_URL + "/exec/" + exec_id + "/json";

    http::resp_t response = http::get(URL);

    switch (response.code) {
    case 200:
        /* No error*/
        break;

    case 404:
        std::cout << "Error in docker::exec_inspect: No such exec instance" << std::endl;
        exit(1);

    case 500:
        std::cout << "Error in docker::exec_inspect: Server error" << std::endl;
        exit(1);

    default:
        std::cout << "Error in docker::exec_inspect: Unknown error" << std::endl;
        exit(1);
    }

    JSON j = JSON::parse(response.body);

    return j;
}

std::string resource_get(std::string container_id, std::string resource_path) {

    std::string URL = BASE_URL + "/containers/" + container_id + "/archive?path=" + resource_path;

    http::resp_t response = http::get(URL, {"Content-Type:application/x-tar"});

    switch (response.code) {
    case 200:
        /* No error*/
        break;

    case 400:
        std::cout << "Error in docker::resource_get: Bad parameter" << std::endl;
        exit(1);

    case 404:
        std::cout << "Error in docker::resource_get: Container or path does not exist" << std::endl;
        exit(1);

    case 500:
        std::cout << "Error in docker::resource_get: Server error" << std::endl;
        exit(1);

    default:
        std::cout << "Error in docker::resource_get: Unknown error" << std::endl;
        exit(1);
    }

    std::filesystem::path p(resource_path);

    std::string tmp_tar_file = TMP_PATH + "/tmp.tar.gz";

    write_file(tmp_tar_file, response.body);

    std::string output_path = TMP_PATH + "/";

    untar(tmp_tar_file, output_path);

    return output_path;
}

}; // namespace docker