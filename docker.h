/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <string>

#include <cstring>

#include <iostream>

#include "global.h"
#include "json.h"
#include "network/HTTP.h"
#include "utils/tar.h"

// #include "filesys.h"

namespace docker {

const std::string protocol = "http";

const std::string host = "localhost";

const int port = 2375;

const std::string BASE_URL = protocol + "://" + host + ":" + std::to_string(port);

bool check_docker_http_api();

std::string image_build(std::string folder_path, std::string image_name);

std::string container_create(std::string image_name, nlohmann::json aditional_opts = {});

std::string container_create_tty(std::string image_name);

std::string container_start(std::string container_id);

std::string exec_create(std::string container_id, std::string cmd);

std::string exec_start(std::string exec_id);

nlohmann::json exec_inspect(std::string exec_id);

std::string resource_get(std::string container_id, std::string resource_path);

}; // namespace docker

// Get an archive of a filesystem resource in a container

// Create an exec instance: Run a command inside a running container.