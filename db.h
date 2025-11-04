/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <filesystem>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <string.h>

#include <vector>

#include <sqlite3.h>

#include "data.h"
#include "utils/debug.h"
#include "utils/utils.h"

int callback(void *results, int count, char **data, char **columns);

class grDB
{

private:
    std::string dbPath;
    sqlite3 *db;

    char *error;

    int sqlite3_bind(sqlite3_stmt *stmt, data::TypedTable &table, char **error)
    {

        for (auto row : table.data())
        {

            int i = 0;
            for (auto field : row)
            {

                switch (table.header()[i++].second)
                {

                case data::RECORD_TYPE::TEXT:
                case data::RECORD_TYPE::BLOB:
                {

                    std::string f = std::get<std::string>(field);

                    if (sqlite3_bind_text(stmt, i, f.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
                    {
                        *error = strdup("sqlite3_bind_text error");
                        return -1;
                    }
                    break;
                }

                case data::RECORD_TYPE::REAL:
                {
                    double f = std::get<double>(field);

                    if (sqlite3_bind_double(stmt, i, f) != SQLITE_OK)
                    {
                        *error = strdup("sqlite3_bind_double error");
                        return -1;
                    }
                    break;
                }

                case data::RECORD_TYPE::INTEGER:
                {
                    int f = std::get<int>(field);

                    if (sqlite3_bind_int(stmt, i, f) != SQLITE_OK)
                    {
                        *error = strdup("sqlite3_bind_int error");
                        return -1;
                    }
                    break;
                }
                }
            }

            int err = sqlite3_step(stmt);
            if (err != SQLITE_DONE)
            {

                if (err == SQLITE_BUSY)
                {
                    *error = strdup("SQLITE BUSY");
                }
                else if (err == SQLITE_ERROR)
                {
                    *error = strdup("SQLITE RUNTIME ERROR");
                }
                else if (err == SQLITE_MISUSE)
                {
                    *error = strdup("SQLITE MISUSE");
                }
                else if (err == SQLITE_CONSTRAINT)
                {
                    *error = strdup("SQLITE CONSTRAINT");
                }

                std::cout << "sqlite3_step error : " << error << std::endl;
                return -1;
            }

            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);
        }

        return SQLITE_OK;
    }

public:
    typedef std::variant<std::string, int, double> field_t;

    enum class ERROR
    {
        OK,
        FILE_NOT_FOUND,
        WRONG_FILE_FORMAT,
        INTERNAL_ERROR
    };

    grDB() {}

    ERROR open(std::string path);

    bool create(std::string path);

    ~grDB()
    {
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
        if (error)
        {
            free(error);
            error = nullptr;
        }
    }

    struct make_string_functor
    {
        std::string operator()(const std::string &x) const { return x; }
        std::string operator()(int x) const { return std::to_string(x); }
        std::string operator()(double x) const { return std::to_string(x); }
    };

    static std::string str(field_t field)
    {

        std::stringstream stream;

        stream << std::visit(make_string_functor(), field);

        return stream.str();
    }

    bool create_table(std::string table_name, data::TypedTable table, std::string primary_key);

    bool insert(std::string table_name, data::TypedTable table, std::string primary_key = "");

    bool update(std::string table_name, std::string column_name, std::string value, std::string where_clause = "");

    std::vector<std::vector<std::string>> table_info(std::string table_name);

    bool table_exists(std::string table_name);

    data::TypedTable dump_table(std::string table_name, std::vector<std::string> columns = {}, std::string where_clause = "");

    data::TypedTable dump_table_OLD(std::string table_name, std::vector<std::string> columns = {}, std::string where_clause = "");

    data::TypedTable select_rows(std::string table_name, std::string where_clause) { return dump_table(table_name, {}, where_clause); }

    std::vector<field_t> select_column(std::string table_name, std::string column_name, std::string where_clause = "");
};

// grDB::field_t

template <typename T>
bool operator==(const grDB::field_t &v, T const &t) { return std::get<T>(v) == t; }

inline std::ostream &operator<<(std::ostream &os, const grDB::field_t &v)
{
    std::visit([&os](auto const &x)
               { os << x; }, v);
    return os;
}
