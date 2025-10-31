/* SPDX-License-Identifier: AGPL-3.0-only */
#include "llvm-cov.h"

LCOVfile::LCOVfile(const std::string filepath) {

    size_t size = 0;
    char *file = read_file(filepath, size);
    if (file == nullptr) {
        return;
    }

    // Get the file from filepath
    std::string tmp = filepath;
    filename = tmp.substr(tmp.find_last_of("/\\") + 1);

    // Remove .html
    filename = filename.substr(0, filename.find_last_of("."));

    // parse html file
    htmlDocPtr doc =
        htmlReadMemory(file, size, filepath.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
    if (doc == NULL) {
        std::cout << "Error: unable to parse html file " << filepath << std::endl;
        return;
    }

    // get root element
    xmlNode *root = xmlDocGetRootElement(doc);

    parse_path(root);

    if (path.empty()) {
        std::cout << "Error: unable to parse path from html file " << filepath << std::endl;
        return;
    }

    parse_lines(root);

    // free the document
    xmlFreeDoc(doc);

    // free the global variables that may have been allocated by the parser
    xmlCleanupParser();

    // this is to debug memory for regression tests
    xmlMemoryDump();
}

std::vector<LCOVfunction> *LCOVfile::scan_for_functions() {

    std::vector<LCOVfunction> *functions = new std::vector<LCOVfunction>();

    // Temporary output file
    std::string ext = this->filename.substr(this->filename.find_last_of(".") + 1);

    std::string tmpFilename = "/tmp/temp_srcFile." + ext;

    // Create output file
    std::ofstream output_file;
    output_file.open(tmpFilename);

    // for auto line : lines
    for (int i = 1; i < lines.size(); i++) {

        output_file << lines[i].source << std::endl;
    }

    output_file.close();

    std::remove("/tmp/temp_outFile");

    char *argv[] = {"/usr/bin/ctags", "-x", "-f", "/tmp/temp_outFile", const_cast<char *>(tmpFilename.c_str()), NULL};

    execute(argv);

    // Open /tmp/temp_outFile
    std::ifstream input_file("/tmp/temp_outFile");

    // Read line by line
    std::string line;
    while (std::getline(input_file, line)) {

        // Get whitespace delimited fields

        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> fields;

        while (ss >> token) {
            fields.push_back(token);
        }

        if (fields.size() > 2) {

            std::string name = fields[0];
            std::string kind = fields[1];
            std::string lineNum = fields[2];

            if (kind != "function") {
                continue;
            }

            // std::cout << name << " " << kind << " " << lineNum << std::endl;

            // Load file into memory
            std::ifstream input_file(tmpFilename);
            // check if file is open
            if (!input_file.is_open()) {
                std::cerr << "Error opening file " << this->filename << std::endl;
                return NULL;
            }

            int lNum = 0;
            try {
                lNum = std::stoi(lineNum);
            } catch (const std::invalid_argument &ia) {
                std::cerr << "Invalid argument: " << ia.what() << std::endl;
            }

            // Read until lineNum
            std::string line;
            int lineCount = 0;
            while (std::getline(input_file, line)) {
                lineCount++;
                if (lineCount >= (lNum - 1)) {
                    break;
                }
            }

            int openBr = 0;
            size_t pos2 = 0;
            size_t pos;

            text_pos body_start = {0, 0};

            while (std::getline(input_file, line)) {

                lineCount++;

                // Search for the start of body function looking for "{"
                pos = line.find("{");
                if (pos != std::string::npos) {
                    openBr = 1;
                    body_start.line += lineCount - lNum;
                    body_start.pos = pos;
                    break;
                }
            }

            for (std::string::iterator it = line.begin() + pos + 1; it < line.end(); ++it) {
                if (*it == '{') {
                    openBr++;
                } else if (*it == '}') {
                    openBr--;
                }
                if (openBr == 0) {
                    pos2 = lineCount;
                    break;
                }
            }

            while (std::getline(input_file, line)) {

                lineCount++;

                for (std::string::iterator it = line.begin(); it < line.end(); ++it) {

                    if (*it == '{') {
                        openBr++;
                    } else if (*it == '}') {
                        openBr--;
                    }
                    if (openBr == 0) {
                        pos2 = lineCount;
                        break;
                    }
                }

                if (pos2) {
                    break;
                }
            }

            if (pos2) {
                // std::cout << "Function starts at line " << lNum << " Function ends at line " << pos2 << std::endl;
            } else {
                std::cout << "Error: unable to find function end" << std::endl;
                return NULL;
            }

            LCOVfunction func(name, path, body_start);

            for (int i = lNum; i <= pos2; i++) {
                func.push(lines[i]);
            }

            functions->push_back(func);

            // search close bracket
        }
    }

    return functions;
}

void LCOVfile::parse_path(xmlNode *root) {

    xmlNode *div_class = find_html_element(root, "div", "class", "source-name-title");

    if (div_class) {
        xmlNode *pre = div_class->children;
        if (pre && strcmp((char *)pre->name, "pre") == 0) {
            xmlNode *text = pre->children;
            if (text) {
                path = (char *)text->content;
            }
        }
    }
}

void LCOVfile::parse_lines(xmlNode *root) {

    xmlNode *div_class = find_html_element(root, "div", "class", "centered");

    if (div_class == NULL) {
        std::cout << "div class centered not found" << std::endl;
        return;
    }

    xmlNode *table = find_html_element(div_class, "table");

    if (table == NULL) {
        std::cout << "table not found" << std::endl;
        return;
    }
    /*
            xmlNode *tbody = find_html_element(table, "tbody");

            if(tbody == NULL){
                return;
            }
    */
    bool headerOK = false;

    int columnNumber = 0;

    xmlNode *tr = table->children;
    for (; tr; tr = tr->next) {
        if (tr->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)tr->name, "tr") == 0) {

                LCOVline l;

                xmlNode *td = tr->children;
                for (; td; td = td->next) {
                    if (td->type == XML_ELEMENT_NODE) {
                        if (strcmp((char *)td->name, "td") == 0) {

                            xmlAttr *attr = td->properties;

                            // First line is header
                            if (attr == NULL) {
                                xmlNode *pre = td->children;
                                if (pre && strcmp((char *)pre->name, "pre") == 0) {
                                    xmlNode *text = pre->children;
                                    if (text) {
                                        std::string content = (char *)text->content;

                                        if (columnNumber == 0 && content == "Line") {
                                            columnNumber++;
                                        } else if (columnNumber == 1 && content == "Count") {
                                            columnNumber++;
                                        } else if (columnNumber == 2 && (content == "Source (" || content == "Source")) {
                                            // Element 0 is header
                                            l.count = 0;
                                            l.line = 0;
                                            l.source = "HEADER";
                                            lines.push_back(l);

                                            columnNumber = 0;
                                            headerOK = true;
                                        } else {
                                            std::cout << "Header not OK" << std::endl;
                                            return;
                                        }

                                        // std::cout << (char *)text->content << std::endl;
                                    }
                                }
                            } else {
                                if (headerOK && strcmp((char *)attr->name, "class") == 0) {
                                    xmlChar *val = xmlNodeListGetString(td->doc, attr->children, 1);
                                    if (columnNumber == 0 && strcmp((char *)val, "line-number") == 0) {

                                        xmlNode *a = td->children;
                                        if (a && strcmp((char *)a->name, "a") == 0) {
                                            xmlNode *pre = a->children;
                                            if (pre && strcmp((char *)pre->name, "pre") == 0) {
                                                xmlNode *text = pre->children;
                                                if (text) {
                                                    l.line = atoi((char *)text->content);
                                                }
                                            }
                                        }
                                        columnNumber++;
                                    } else if (columnNumber == 1 &&
                                               (strcmp((char *)val, "uncovered-line") == 0 || strcmp((char *)val, "covered-line") == 0)) {
                                        xmlNode *pre = td->children;
                                        if (pre && strcmp((char *)pre->name, "pre") == 0) {
                                            xmlNode *text = pre->children;
                                            if (text) {
                                                std::string tmp = (char *)text->content;
                                                l.count = std::stoi(tmp);

                                                if (tmp.back() == 'k') {
                                                    l.count = std::stof(tmp) * 1000;
                                                } else if (tmp.back() == 'm') {
                                                    l.count = std::stof(tmp) * 1000000;
                                                } else {
                                                    l.count = std::stoi(tmp);
                                                }
                                            }
                                        }
                                        columnNumber++;
                                    } else if (columnNumber == 2 && strcmp((char *)val, "code") == 0) {
                                        xmlNode *pre = td->children;
                                        if (pre && strcmp((char *)pre->name, "pre") == 0) {

                                            xmlNode *span = pre->children;
                                            for (; span; span = span->next) {

                                                if (span->type == XML_ELEMENT_NODE) {

                                                    if (strcmp((char *)span->name, "span") == 0) {
                                                        xmlAttr *attr = span->properties;
                                                        if (attr && strcmp((char *)attr->name, "class") == 0) {
                                                            xmlChar *val = xmlNodeListGetString(span->doc, attr->children, 1);
                                                            if (strcmp((char *)val, "red") == 0) {
                                                                xmlNode *text = span->children;
                                                                if (text) {
                                                                    l.source += (char *)text->content;
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    l.source += (char *)span->content;
                                                }
                                            }

                                            lines.push_back(l);

                                            columnNumber = 0;
                                        }
                                    } else {
                                        std::cout << "Line is not OK" << std::endl;
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

coverage LLVM_COV::get_global_coverage(std::string html_report_file) {

    coverage c;

    c.functions_cov = 0;
    c.functions = 0;
    c.lines_cov = 0;
    c.lines = 0;
    c.regions_cov = 0;
    c.regions = 0;

    // parse html file
    htmlDocPtr doc = htmlReadFile(html_report_file.c_str(), NULL,
                                  HTML_PARSE_NOBLANKS | HTML_PARSE_NONET | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
    if (doc == NULL) {
        std::cout << "Error: unable to parse html file " << html_report_file << std::endl;
        exit(EXIT_FAILURE);
    }

    // get root element
    xmlNode *root = xmlDocGetRootElement(doc);

    // Find all div class="source-name-title" in libxml2

    std::vector<xmlNode *> table_nodes = find_all_html_elements(root, "table");

    bool finished = false;

    for (auto table : table_nodes) {

        if (finished) {
            break;
        }

        xmlNode *thead = find_html_element(table, "thead");

        std::vector<xmlNode *> th = find_all_html_elements(thead, "th");

        int path = -1;
        int line_cov = -1;
        int func_cov = -1;
        int region_cov = -1;

        // Check if the table contains "PATH", "LINE COVERAGE", "FUNCTION COVERAGE" and "REGION COVERAGE"
        // for(auto header : th){

        for (int i = 0; i < th.size(); i++) {

            xmlNode *header = th[i];

            xmlChar *xmlVal = xmlNodeListGetString(header->doc, header->xmlChildrenNode, 1);

            // Remove /n and whitespaces
            const char *val = strdup(trim(std::string((char *)xmlVal)).c_str());

            if (strcasecmp(val, "PATH") == 0) {
                path = i;
            } else if (strcasecmp(val, "LINE COVERAGE") == 0) {
                line_cov = i;
            } else if (strcasecmp(val, "FUNCTION COVERAGE") == 0) {
                func_cov = i;
            } else if (strcasecmp(val, "REGION COVERAGE") == 0) {
                region_cov = i;
            }
        }

        if (path < 0 || line_cov < 0 || func_cov < 0 || region_cov < 0) {
            break;
        }

        xmlNode *tbody = find_html_element(table, "tbody");
        if (tbody == NULL) {
            break;
        }

        xmlNode *tfoot = find_html_element(tbody, "tfoot");
        if (tfoot == NULL) {
            break;
        }

        std::vector<xmlNode *> td = find_all_html_elements(tfoot, "td");

        /*
        if(td.size() != 8){
            break;
        }
        */

        int max = std::max(path, line_cov);
        max = std::max(max, func_cov);
        max = std::max(max, region_cov);

        if (max >= td.size()) {
            std::cout << "Debug: Error: Malformed report table" << std::endl;
            break;
        }

        // for(auto data : td){

        for (int n = 0; n < 3; n++) {

            int i = 0;

            if (n == 0) {
                i = line_cov;
            } else if (n == 1) {
                i = func_cov;
            } else if (n == 2) {
                i = region_cov;
            }

            xmlNode *data = td[i];

            xmlNode *pre = find_html_element(data, "pre");

            xmlNode *child = pre->children;

            xmlChar *val = xmlNodeListGetString(pre->doc, pre->xmlChildrenNode, 1);

            std::cout << val << std::endl;

            auto result = parse_coverage_field((char *)val);

            if (!result) {
                break;
            }

            if (n == 0) {
                c.lines = result->second;
                c.lines_cov = result->first;
            } else if (n == 1) {
                c.functions = result->second;
                c.functions_cov = result->first;
            } else if (n == 2) {
                c.regions = result->second;
                c.regions_cov = result->first;

                finished = true;
            }
        }
    }

    // xmlNode *div_class = find_html_element(root, "div", "class", "source-name-title");

    // xmlNode *table = find_html_element(div_class, "table");

    // free the document
    xmlFreeDoc(doc);

    // free the global variables that may have been allocated by the parser
    xmlCleanupParser();

    // Check if those strings are inside a 4 column table

    "column-entry-green";
    "column-entry-red";
    "column-entry-yellow";

    return c;

error: {
    std::cout << "Error: unable to parse html file " << html_report_file << std::endl;
    exit(EXIT_FAILURE);
}
}

std::unique_ptr<std::pair<int, int>> LLVM_COV::parse_coverage_field(std::string field) {

    auto result = std::make_unique<std::pair<int, int>>();

    size_t openPar = field.find("(");

    if (openPar == std::string::npos) {
        return nullptr;
    }

    size_t closePar = field.find(")", openPar);

    if (closePar == std::string::npos) {
        return nullptr;
    }

    field = field.substr(openPar + 1, closePar - openPar - 1);

    size_t div = field.find("/");

    if (div == std::string::npos) {
        return nullptr;
    }

    std::string covered = field.substr(0, div);
    std::string total = field.substr(div + 1);

    try {
        result->first = std::stoi(covered);
    } catch (std::invalid_argument const &ex) {
        std::cout << "Invalid covered argument: " << ex.what() << '\n';
    }

    try {
        result->second = std::stoi(total);
    } catch (std::invalid_argument const &ex) {
        std::cout << "Invalid total argument: " << ex.what() << '\n';
    }

    return result;

error:
    return nullptr;
}

xmlNode *find_html_element(xmlNode *node, const char *tag) {

    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)cur_node->name, tag) == 0) {
                return cur_node;
            }
        }
        xmlNode *found = find_html_element(cur_node->children, tag);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

xmlNode *find_html_element(xmlNode *node, const char *tag, const char *attribute, const char *value) {

    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)cur_node->name, tag) == 0) {
                xmlAttr *attr = cur_node->properties;
                while (attr != NULL) {
                    if (strcmp((char *)attr->name, attribute) == 0) {
                        // Get attribute value
                        xmlChar *val = xmlNodeListGetString(cur_node->doc, attr->children, 1);
                        if (strcmp((char *)val, value) == 0) {
                            return cur_node;
                        }
                    }
                    attr = attr->next;
                }
            }
        }
        xmlNode *found = find_html_element(cur_node->children, tag, attribute, value);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

std::vector<xmlNode *> find_all_html_elements(xmlNode *node, const char *tag) {

    std::vector<xmlNode *> elements;

    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)cur_node->name, tag) == 0) {
                elements.push_back(cur_node);
            }
        }
        std::vector<xmlNode *> found = find_all_html_elements(cur_node->children, tag);
        if (found.size() > 0) {
            elements.insert(elements.end(), found.begin(), found.end());
        }
    }
    return elements;
}
