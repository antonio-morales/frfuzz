/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <list>

#include "campaign.h"
#include "global.h"
#include "utils/process.h"

void initialize();

std::list<std::string> load_projects();