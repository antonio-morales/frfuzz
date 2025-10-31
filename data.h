/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <vector>

#include <string>

#include <cstdint>

#include <sstream>

#include <iostream>

#include <variant>

namespace data {

enum TYPE { TEXT, LINK, TABLE, FIXEDTABLE, TYPEDTABLE, CODEBLOCK, SCRIPT };

class Object {
  public:
    virtual TYPE type() const = 0;
};

class Text : public Object {

  private:
    std::vector<std::pair<uint32_t, uint32_t>> prvHighlight;

  protected:
    std::string text;

  public:
    Text() {}

    Text(std::string text) { this->text = text; }

    ~Text() {}

    TYPE type() const { return TYPE::TEXT; }

    const std::string &data() const { return text; }

    void highlight(std::string subStr) {

        // Search in text
        // Replace with <span class="highlight">str</span>

        int index = 0;
        while ((index = text.find(subStr, index)) != std::string::npos) {

            prvHighlight.emplace_back(index, index + subStr.length());

            index += subStr.length();
        }
    }

    const std::vector<std::pair<uint32_t, uint32_t>> &highlight() const { return prvHighlight; }
};

class Script : public Object {

  private:
  protected:
    std::string code;

  public:
    Script() {}

    Script(std::string code) { this->code = code; }

    ~Script() {}

    TYPE type() const { return TYPE::SCRIPT; }

    const std::string &data() const { return code; }
};

class CodeBlock : public Text {
  private:
    std::string language;

  public:
    CodeBlock() {}

    CodeBlock(std::string text) { this->text = text; }

    ~CodeBlock() {}

    TYPE type() const { return TYPE::CODEBLOCK; }

    /*
        void highlight(std::string str){

        }
    */
};

class Link : public Object {
  private:
    std::string text;
    std::string url;

  public:
    Link(std::string text, std::string url) {
        this->text = text;
        this->url = url;
    }

    ~Link() {}

    TYPE type() const { return TYPE::LINK; }
};

class Table : public Object {
  private:
    std::vector<std::string> prvHeader;

    // TODO: Links? Images?
    std::vector<std::vector<std::vector<Object *>>> rows;

    uint32_t numRows = 1;

    uint32_t currCol = 0;

  public:
    Table();

    ~Table();

    TYPE type() const { return TYPE::TABLE; }

    void setHeader(const std::initializer_list<std::string> &columnName = {}) {
        for (auto c : columnName) {
            prvHeader.push_back(c);
        }
    }

    void newColumn();

    void newRow();

    const std::vector<std::string> &header() const;

    const std::vector<std::vector<std::vector<Object *>>> &data() const;

    // operator << overload
    template <typename T> Table &operator<<(const T &t) {

        std::stringstream buffer;

        buffer << t;

        data::Text *text = new data::Text(buffer.str());

        rows[numRows - 1][currCol].push_back(text);

        return *this;
    }

    // operator << overload
    // template <typename T>
    Table &operator<<(const CodeBlock &cb) {

        data::CodeBlock *ptr = new data::CodeBlock(cb);

        // data::Object *object = new data::Object(o);

        rows[numRows - 1][currCol].push_back(ptr);

        return *this;
    }

    std::string str();
};

enum class RECORD_TYPE { TEXT, REAL, INTEGER, BLOB };

class TypedTable : public Object {

  private:
    std::vector<std::pair<std::string, RECORD_TYPE>> prvHeader;

    std::vector<std::vector<std::variant<std::string, int, double>>> rows;

    uint32_t numColumns = 0;

    uint32_t currCol = 0;

  public:
    TypedTable(const std::initializer_list<std::pair<std::string, RECORD_TYPE>> &field = {}) {
        for (auto f : field) {
            prvHeader.push_back(f);
            numColumns++;
        }
    }

    ~TypedTable() {}

    bool empty() { return rows.size() == 0; }

    void newColumn(std::pair<std::string, RECORD_TYPE> c) {
        prvHeader.push_back(c);
        numColumns++;
    }

    TYPE type() const { return TYPE::TYPEDTABLE; }

    int getNumberColumns() { return numColumns; }

    void insert(std::vector<std::variant<std::string, int, double>> row) {

        if (row.size() != numColumns) {
            throw std::runtime_error("TypedTable::insert: row size does not match number of columns");
        }

        for (int i = 0; i < numColumns; i++) {
        }

        bool error = false;
        int c = 0;
        for (auto &r : row) {

            if (error) {
                break;
            }

            std::visit(
                [this, &c, &error](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<T, std::string>) {
                        if (prvHeader[c].second != RECORD_TYPE::TEXT && prvHeader[c].second != RECORD_TYPE::BLOB) {
                            // throw std::runtime_error("TypedTable::insert: type mismatch");
                            std::cout << "TypedTable::insert: type mismatch: TEXTO " << arg << '\n';
                            error = true;
                            return;
                        }
                        std::cout << "Debug : String arg: " << arg << '\n';

                    } else if constexpr (std::is_same_v<T, int>) {
                        if (prvHeader[c].second != RECORD_TYPE::INTEGER) {
                            // throw std::runtime_error("TypedTable::insert: type mismatch");
                            std::cout << "TypedTable::insert: type mismatch: INTEGER " << arg << '\n';
                            error = true;
                            return;
                        }
                        std::cout << "Debug : Int arg: " << arg << '\n';

                    } else if constexpr (std::is_same_v<T, double>) {
                        if (prvHeader[c].second != RECORD_TYPE::REAL) {
                            // throw std::runtime_error("TypedTable::insert: type mismatch");
                            std::cout << "TypedTable::insert: type mismatch: REAL " << arg << '\n';
                            error = true;
                            return;
                        }
                        std::cout << "Debug : Double arg: " << arg << '\n';
                    }
                },
                r);

            c++;
        }

        if (!error) {
            rows.push_back(row);
        }
    }

    void insert_fromString(std::vector<std::string> input_row) {

        if (input_row.size() != numColumns) {
            throw std::runtime_error("TypedTable::insert_fromString: input_row size does not match the number of columns of the table");
        }

        std::vector<std::variant<std::string, int, double>> row;

        for (int i = 0; i < numColumns; i++) {

            if (prvHeader[i].second == data::RECORD_TYPE::TEXT || prvHeader[i].second == data::RECORD_TYPE::BLOB) {
                row.push_back(input_row[i]);

            } else if (prvHeader[i].second == data::RECORD_TYPE::INTEGER) {
                row.push_back(std::stoi(input_row[i]));

            } else if (prvHeader[i].second == data::RECORD_TYPE::REAL) {
                row.push_back(std::stod(input_row[i]));
            }
        }

        rows.push_back(row);
    }

    const std::vector<std::pair<std::string, RECORD_TYPE>> &header() const { return prvHeader; }

    const std::vector<std::vector<std::variant<std::string, int, double>>> &data() const { return rows; }

    // operator << overload
    template <typename T> TypedTable &operator<<(const T &t) {

        if (rows.size() == 0) {

            // Add first row
            rows.push_back({});

            // Fixed size
            rows[0].resize(numColumns);
        }

        if (currCol == numColumns) {
            currCol = 0;

            // Add new row
            rows.push_back({});

            rows.back().resize(numColumns);
        }

        std::stringstream buffer;
        buffer << t;

        rows[rows.size() - 1][currCol] = buffer.str();

        return *this;
    }

    std::string str();
};

} // namespace data