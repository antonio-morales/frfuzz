/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <cstring>
#include <iostream>
#include <string>

// include libxml2
#include <libxml/HTMLparser.h>

#include "html.h"

namespace html {

class Parser {
  private:
    htmlDocPtr doc;

    xmlNode *root = NULL;

    void conc__find_all_element(xmlNode *node, const char *tag, std::vector<xmlNode *> &elements);

  public:
    Parser(std::string html_buffer, std::string baseURL);

    ~Parser() {
        if (doc != NULL) {
            xmlFreeDoc(doc);
        }
    }

    std::vector<xmlNode *> find_all_elements(const char *tag);

    std::vector<html::Hyperlink> get_hyperlinks();
};

} // namespace html