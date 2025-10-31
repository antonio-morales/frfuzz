/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <yara.h>

#include "utils/filesys.h"

int yc_load_rules(const char *rule_path, YR_RULES **out_rules, char *errbuf, size_t errbuf_sz);

int _match_cb(YR_SCAN_CONTEXT *ctx, int message, void *message_data, void *user_data);