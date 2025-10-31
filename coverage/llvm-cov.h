/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <fstream>
#include <iostream>

#include <cstring>

#include "coverage.h"

#include "utils/filesys.h"

#include "../data.h"

#include "utils/process.h"

// #include "../utils.h"

// include libxml2
#include <libxml/HTMLparser.h>

xmlNode *find_html_element(xmlNode *node, const char *tag);

xmlNode *find_html_element(xmlNode *node, const char *tag, const char *attribute, const char *value);

std::vector<xmlNode *> find_all_html_elements(xmlNode *node, const char *tag);

struct text_pos {
    uint line;
    size_t pos;
};

class LCOVfile {

  private:
    std::string filename;
    std::string path;

    std::vector<LCOVline> lines;

    // std::vector<LCOVfunction> functions;

    void parse_path(xmlNode *root);

    void parse_lines(xmlNode *root);

  public:
    LCOVfile(const std::string filepath);

    /*
            std::size_t found = file.find("<div class=\"source-name-title\">");
            if (found != std::string::npos){

            }

            std::string "<div class=\"source-name-title\">"

    <pre>/src/c-ares/src/lib/ares__parse_into_addrinfo.c</pre></div>
        }
        */
    ~LCOVfile() {}

    std::vector<LCOVfunction> *scan_for_functions();
};

class LLVM_COV {

  private:
    static std::unique_ptr<std::pair<int, int>> parse_coverage_field(std::string field);

  public:
    static coverage get_global_coverage(std::string html_report_file);
};
