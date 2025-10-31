/* SPDX-License-Identifier: AGPL-3.0-only */
#include "validator/yara.h"

static int _compiler_cb(int error_level, const char *file_name, int line_number, const YR_RULE *rule, const char *message, void *user_data) {
    (void)rule;
    if (!user_data || !message)
        return 0;
    char *buf = (char *)user_data;
    size_t cap = *(size_t *)(((void **)user_data) + 1); // small trick avoided; keep simple instead
    (void)cap;
    // Simpler: just print to stderr; or pass a small buffer from caller.
    fprintf(stderr, "[YARA] %s:%d: %s\n", file_name ? file_name : "(input)", line_number, message);
    return 0;
}

int yc_load_rules(const char *rule_path, YR_RULES **out_rules, char *errbuf, size_t errbuf_sz) {

    if (!out_rules || !rule_path) {
        return ERROR_INTERNAL_FATAL_ERROR;
    }

    *out_rules = NULL;

    YR_COMPILER *comp = NULL;
    int rc = yr_compiler_create(&comp);
    if (rc != ERROR_SUCCESS) {
        return rc;
    }

    /*
    if (errbuf && errbuf_sz) {
        // Imprime errores a stderr; si quieres acumular en errbuf implementa un buffer propio.
        yr_compiler_set_callback(comp, _compiler_cb, NULL);
    }
    */

    std::string rule_text = read_file(rule_path);

    int errs = yr_compiler_add_string(comp, rule_text.c_str(), NULL);
    if (errs > 0) {
        fprintf(stderr, "Compilation failed: %d error(s)\n", errs);
        yr_compiler_destroy(comp);
        yr_finalize();
        return 1;
    }

    YR_RULES *rules = NULL;
    rc = yr_compiler_get_rules(comp, &rules);
    yr_compiler_destroy(comp);
    if (rc != ERROR_SUCCESS)
        return rc;

    *out_rules = rules;
    return ERROR_SUCCESS;
}

int _match_cb(YR_SCAN_CONTEXT *ctx, int message, void *message_data, void *user_data) {
    (void)ctx;
    (void)message_data;
    if (message == CALLBACK_MSG_RULE_MATCHING && user_data) {
        int *hit = static_cast<int *>(user_data);
        *hit = 1;
    }
    return CALLBACK_CONTINUE;
}