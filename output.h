/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "data.h"
#include "error.h"
#include "html/html.h"

enum OutFormat { TEXT, HTML };

class customOutput {
  private:
    std::vector<data::Object *> buffer;

    // std::stringstream buffer;

    OutFormat format = TEXT;

    std::ofstream outFile;
    std::string currentPath = "";

    // std::unordered_map<std::string, std::string> htmlOptions;

    struct htmlOptions {
        std::string *title = NULL;
        std::string *style = NULL;
    };

    struct stOptions {
        std::unordered_map<std::string, std::string> html;
    } options;

  public:
    customOutput() {}

    customOutput(OutFormat f) { format = f; }

    ~customOutput() {
        // file.close();

        // delete options.html;
    }

    void setFormat(OutFormat f) { format = f; }

    // TODO: Improve this
    bool setOption(std::string group, std::string option, std::string value) {

        if (group == "html") {
            options.html[option] = value;
        } else {
            return false;
        }

        return true;
    }

    bool setFile(std::string path) {

        currentPath = path;

        return true;
    }

    bool clear() {

        // Clear the pointer vector
        for (auto obj : buffer) {
            delete obj;
        }

        buffer.clear();
        return true;
    }

    bool dump();

    void write(std::string str) {
        // file << str;
    }

    void print() {
        // TODO
        // std::cout << buffer.str() << std::endl;
    }

    /*
        friend std::ostream &operator<<(std::__cxx11::basic_stringstream<char> &stream,  const Table &table)
        {

            stream << "table";

            //Traverse rows
            for (auto row : table.data())
            {
                //Traverse columns
                for (auto column : row)
                {
                    //Traverse characters
                    for (auto c : column)
                    {
                        stream << c;
                    }

                    stream << " | ";
                }

                stream << std::endl;

            }

            return stream;
        }
    */
    // overload operator << for stringstream with a friend
    // friend

    template <typename T> customOutput &operator<<(const T &t) {
        buffer << t;
        return *this;
    }

    customOutput &operator<<(const data::Table &t) {

        buffer.push_back(new data::Table(t));

        return *this;
    }
};