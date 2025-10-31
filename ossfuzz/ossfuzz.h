/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <string>

#include "network/HTTP.h"

#include "yaml/yaml.h"

namespace ossfuzz {

std::string get_repo_url(std::string project);

}