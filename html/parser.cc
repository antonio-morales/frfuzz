/* SPDX-License-Identifier: AGPL-3.0-only */
#include "parser.h"

namespace html {

void Parser::conc__find_all_element(xmlNode *node, const char *tag, std::vector<xmlNode *> &elements) {

    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)cur_node->name, tag) == 0) {
                elements.push_back(cur_node);
                // return;
            }
        }
        conc__find_all_element(cur_node->children, tag, elements);
    }
}

Parser::Parser(std::string html_buffer, std::string baseURL) {

    // parse html file
    doc = htmlReadMemory(html_buffer.c_str(), html_buffer.size(), baseURL.c_str(), NULL,
                         HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

    if (doc == NULL) {
        std::cout << "Error: unable to parse html file" << std::endl;
        return;
    }

    root = xmlDocGetRootElement(doc);
}

std::vector<xmlNode *> Parser::find_all_elements(const char *tag) {

    std::vector<xmlNode *> elements;

    conc__find_all_element(root, tag, elements);

    return elements;
}

std::vector<html::Hyperlink> Parser::get_hyperlinks() {

    std::vector<html::Hyperlink> hyperlinks;

    std::vector<xmlNode *> elements = find_all_elements("a");

    for (auto elem : elements) {

        xmlAttr *attr = elem->properties;
        while (attr != NULL) {
            if (strcmp((char *)attr->name, "href") == 0) {
                // Get attribute value
                xmlChar *href = xmlNodeListGetString(elem->doc, attr->children, 1);
                xmlChar *text = xmlNodeListGetString(elem->doc, elem->children, 1);

                if (href && href[0] != '#') { // ignore href starts with #
                    html::Hyperlink hl((char *)href, (char *)text);
                    hyperlinks.push_back(hl);
                }

                break;
            }
            attr = attr->next;
        }
    }

    elements = find_all_elements("link");

    for (auto elem : elements) {

        xmlAttr *attr = elem->properties;
        while (attr != NULL) {
            if (strcmp((char *)attr->name, "href") == 0) {
                xmlChar *href = xmlNodeListGetString(elem->doc, attr->children, 1);

                html::Hyperlink hl((char *)href, "");
                hyperlinks.push_back(hl);

                break;
            }
            attr = attr->next;
        }
    }

    return hyperlinks;
}

} // namespace html