/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <X11/Xlib.h>
#include <string>

#include "utils/process.h"

bool get_screen_resolution(size_t &width, size_t &height);

void switch_to_desktop(size_t desktop);

int divRoundClosest(const int n, const int d);