/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <source_location>

#include <iostream>
#include <string>

#include <libgen.h>

void criticalError(const std::source_location location = std::source_location::current());
void criticalError(std::string msg, const std::source_location location = std::source_location::current());
