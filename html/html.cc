/* SPDX-License-Identifier: AGPL-3.0-only */
#include <iostream>

#include "html.h"

namespace html {

std::string escape(std::string input) {
    std::string str;

    for (char &c : input) {

        switch (c) {
        case '<':
            str += "&lt;";
            break;
        case '>':
            str += "&gt;";
            break;
        case '&':
            str += "&amp;";
            break;
        case '\"':
            str += "&quot;";
            break;
        case '\'':
            str += "&apos;";
            break;
        default:
            str += c;
            break;
        }
    }

    return str;
}

std::string link(std::string text, std::string url) {
    std::string str = "<a href=\"" + url + "\">" + text + "</a>";
    return str;
}

std::unordered_map<std::string, std::string> parse_attributes(std::string str_attr) {

    std::unordered_map<std::string, std::string> attributes;

    auto start_attr = str_attr.begin();
    auto current_pos = start_attr;
    auto end_pos = start_attr;

    while (current_pos < str_attr.end()) {

        if (*current_pos == '=') {

            end_pos = current_pos + 1;

            if (end_pos >= str_attr.end()) {
                std::cout << "Error: html::parse_attributes: empty attribute value is not allowed" << std::endl;
                return {};
            }

            if (*end_pos != '"') {
                std::cout << "Error: html::parse_attributes: attribute value must be enclosed in '\"'" << std::endl;
                return {};
            }

            end_pos++;

            while (end_pos < str_attr.end()) {

                if (*end_pos == '"') {

                    std::string attribute = trim(std::string(start_attr, current_pos));

                    std::string value = std::string(current_pos + 2, end_pos);

                    attributes.insert({attribute, value});

                    current_pos = start_attr = end_pos + 1;

                    break;
                }

                end_pos++;
            }

            if (end_pos == str_attr.end()) {
                std::cout << "Error: html::parse_attributes: attribute value must be enclosed in '\"'" << std::endl;
                return {};
            }

            // std::string atribute = std::string(start_attr, tmp.end());

            // break;
        }

        current_pos++;
    }

    // size_t end_pos = tmp.find("=", start_pos);

    return attributes;
}

/*
    std::string print(const Table &t){

        std::string str = "<table>";

        //Traverse rows
        for (auto row : t.data())
        {
            str += "<tr>";
            //Traverse columns
            for (auto column : row)
            {

                str += "<td>";
                //Traverse characters
                for (auto c : column)
                {
                    str += escape(c);
                }
                str += "</td>";
            }
            str += "</tr>";
        }

        str += "</table>";

        return str;

    }
    */

html::Element *convert(const data::Object *obj) {

    if (obj->type() == data::TYPEDTABLE) {

        html::Table *newTable = new html::Table();

        data::TypedTable *origTable = (data::TypedTable *)obj;

        // Set header
        std::vector<std::string> header;

        for (auto h : origTable->header()) {
            header.push_back(h.first);
        }

        newTable->set_header(header);

        std::vector<std::vector<grDB::field_t>> rows;

        // Traverse rows
        for (auto row : origTable->data()) {

            std::vector<std::string> newRow;

            // Traverse columns
            for (auto field : row) {

                std::string str_field = grDB::str(field);

                newRow.push_back(str_field);
            }

            newTable->add_row(newRow);
        }

        return newTable;

    } else if (obj->type() == data::TABLE) {

        html::Table *newTable = new html::Table();

        data::Table *origTable = (data::Table *)obj;

        // Set header
        newTable->set_header(origTable->header());

        // Traverse rows
        for (auto row : origTable->data()) {

            std::vector<std::string> newRow;

            // Traverse columns
            for (auto column : row) {
                std::string cell;

                for (auto o : column) {

                    html::Element *elem = convert(o);

                    cell += elem->str();
                }

                newRow.push_back(cell);
            }

            newTable->add_row(newRow);
        }

        return newTable;

    } else if (obj->type() == data::TEXT) {

        data::Text *origText = (data::Text *)obj;

        html::Text *newText;

        std::cout << origText->data() << std::endl;

        if (origText->highlight().size() > 0) {

            newText = new html::Text(origText->data(), origText->highlight());

        } else {
            newText = new html::Text(origText->data());
        }

        return newText;

    } else if (obj->type() == data::CODEBLOCK) {

        data::CodeBlock *origCodeBlock = (data::CodeBlock *)obj;

        std::cout << origCodeBlock->data() << std::endl;

        // Div *div = buildTree(Div, Pre, Code, Text);

        Div *div = new Div(new Pre(new Code(new Text(origCodeBlock->data(), origCodeBlock->highlight()))));

        return div;

    } else if (obj->type() == data::SCRIPT) {

        data::Script *origScript = (data::Script *)obj;

        html::Script *newScript = new html::Script(origScript->data());

        return newScript;
    }

    // Does nothing

    return new html::Element();
}

}; // namespace html

std::ostream &operator<<(std::ostream &stream, html::Document &doc) {

    stream << doc.str();

    return stream;
}