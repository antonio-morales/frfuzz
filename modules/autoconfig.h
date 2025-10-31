/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include "github/API.h"
#include "utils/debug.h"

namespace autoconfig {

void list();

bool install(FRglobal &ctx, std::string configuration);

} // namespace autoconfig