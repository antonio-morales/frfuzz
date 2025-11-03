/* SPDX-License-Identifier: AGPL-3.0-only */
#include "db.h"

int callback(void *results, int count, char **data, char **columns) {

    std::vector<std::vector<std::string>> *res = (std::vector<std::vector<std::string>> *)results;

    std::vector<std::string> row;

    int i = 0;
    while (data[i] != NULL) {
        row.push_back(data[i]);
        i++;
    }

    res->push_back(row);

    return 0;
}

grDB::ERROR grDB::open(std::string path) {

    int errcode;

    // We have to open the database
    if ((errcode = sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, NULL)) == SQLITE_OK) {

        debug() << "Database opened" << std::endl;

        // Check if it is a GRFuzz database (Read the fingerprint of the database)
        auto column = select_column("grDB", "fingerprint");
        if (column.size() == 0 || column[0] != 0x67724442) {
            std::cerr << "Error in grDB::open(): grDB fingerprint not found" << std::endl;
            return ERROR::WRONG_FILE_FORMAT;
        }

        if (is_debug_enabled()) {
            std::cerr << "DEBUG: Fingerprint = 0x" << std::hex << column[0] << std::endl;
        }

    } else if (errcode == SQLITE_CANTOPEN) {

        // errcode = sqlite3_extended_errcode(db);
        // std::cout << sqlite3_errmsg(db) << std::endl;

        debug() << "Error in grDB::open: sqlite3_open failed" << std::endl;
        return ERROR::FILE_NOT_FOUND;

    } else {
        debug() << "Error in grDB::open: sqlite3_open failed" << std::endl;
        return ERROR::INTERNAL_ERROR;
    }

    dbPath = path;

    return ERROR::OK;
}

bool grDB::create(std::string path) {

    // Check if file exists with std::filesystem
    if (!std::filesystem::exists(path)) {

        // We have to create the database
        if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) == SQLITE_OK) {
            debug() << "DEBUG: Database created" << std::endl;

        } else {
            std::cerr << "Error in grDB::create: sqlite3_open failed" << std::endl;
            return false;
        }

        // Create GRFuzz database fingerprint

        data::TypedTable fingerprint_table({{"fingerprint", data::RECORD_TYPE::INTEGER}});
        fingerprint_table.insert({0x67724442}); //"grDB" as hex

        if (!insert("grDB", fingerprint_table)) {
            std::cerr << "Error in grDB::create: insert failed" << std::endl;
            return false;
        }

        debug() << "DEBUG: Fingerprint inserted" << std::endl;

    } else {
        std::cerr << "Error in grDB::create(): Database already exists" << std::endl;
        return false;
    }

    dbPath = path;

    return true;
}

bool grDB::create_table(std::string table_name, data::TypedTable table, std::string primary_key) {

    // Create table
    std::string sql = "CREATE TABLE if not exists " + table_name + "(";

    for (auto c : table.header()) {
        sql += c.first + " ";

        switch (c.second) {
        case data::RECORD_TYPE::TEXT:
            sql += "TEXT";
            break;
        case data::RECORD_TYPE::REAL:
            sql += "REAL";
            break;
        case data::RECORD_TYPE::INTEGER:
            sql += "INTEGER";
            break;
        case data::RECORD_TYPE::BLOB:
            sql += "BLOB";
            break;
        }

        sql += ",";
    }

    // Remove last comma
    sql.pop_back();

    if (primary_key != "") {
        sql += ",PRIMARY KEY (" + primary_key + ")";
    }

    /*
                                        "	project         TEXT	NOT NULL," \
                    "	date            TEXT            	NOT NULL," \
                    "	lines_cov       REAL            	NOT NULL," \
                    "	lines           REAL            	NOT NULL," \
                    "	functions_cov   REAL            	NOT NULL," \
                    "	functions       REAL            	NOT NULL," \
                    "	regions_cov     REAL            	NOT NULL," \
                    "	regions         REAL            	NOT NULL," \
                    "	main_repo       TEXT            	," \
                    "	stars           INTEGER            	," \
                    "	forks           INTEGER            	," \
                    "	local_path      TEXT            	," \
                    "   PRIMARY KEY (project, date)" \
                    ");";

    */

    sql += ");";

    debug() << sql << std::endl;

    if (sqlite3_exec(db, sql.c_str(), NULL, NULL, &error) != SQLITE_OK) {
        std::cerr << "sqlite3_exec error: " << error << std::endl;
        return false;
    }

    debug() << "Table created" << std::endl;

    return true;
}

bool grDB::insert(std::string table_name, data::TypedTable table, std::string primary_key) {

    // Create table
    if (!create_table(table_name, table, primary_key)) {
        std::cout << "Error in grDB::insert: create_table failed" << std::endl;
        return false;
    }

    debug() << "Table '" << table_name << "' created" << std::endl;

    sqlite3_stmt *stmt;

    char *tail;

    std::string sql = "INSERT INTO " + table_name + " (";

    for (auto c : table.header()) {
        sql += c.first + ",";
    }
    sql.pop_back();

    sql += ") VALUES (";

    for (auto c : table.header()) {
        sql += "?,";
    }
    sql.pop_back();

    sql += ")";

    debug() << sql << std::endl;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, (const char **)&tail) != SQLITE_OK) {
        printf("sqlite3_prepare_v2 error: %s \n", error);
        exit(-1);
    }

    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);

    if (sqlite3_bind(stmt, table, &error) != SQLITE_OK) {
        printf("sqlite3_bind error: %s \n", error);
        exit(-1);
    }

    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &error);

    sqlite3_finalize(stmt);

    // sqlite3_close(db);

    return true;
}

bool grDB::update(std::string table_name, std::string column_name, std::string value, std::string where_clause) {

    // std::string sql = "UPDATE " + table_name + " SET " + column_name + " = '" + value + "' WHERE " + where_clause + ";";

    /*
    if(sqlite3_exec(db, sql.c_str(), NULL, NULL, &error) != SQLITE_OK){
        std::cerr << "sqlite3_exec error: " << error << std::endl;
        return false;
    }
        */

    /*
    UPDATE table_name
    SET column1 = value1, column2 = value2, ...
    WHERE condition;
    */

    std::string sql = "UPDATE " + table_name + " SET " + column_name + " = ? WHERE " + where_clause + ";";

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
        return rc;

    // 1st placeholder: blob
    rc = sqlite3_bind_blob(stmt, 1, value.data(), value.size(), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

bool grDB::table_exists(std::string table_name) {

    std::string sql = "PRAGMA table_info(" + table_name + ");";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW); // has at least one column
    sqlite3_finalize(stmt);
    return exists;
}

std::vector<std::vector<std::string>> grDB::table_info(std::string table_name) {

    std::string sql = "pragma table_info(" + table_name + ");";

    std::vector<std::vector<std::string>> results;

    if (sqlite3_exec(db, sql.c_str(), callback, &results, &error) != SQLITE_OK) {
        std::cerr << "sqlite3_exec error: " << error << std::endl;
        return {};
    }

    // Print results
    for (auto row : results) {
        for (auto field : row) {
            debug() << field << "|";
        }
        debug() << std::endl;
    }

    return results;
}

data::TypedTable grDB::dump_table(std::string table_name, std::vector<std::string> columns, std::string where_clause) {

    std::string columns_sql = "*";
    if (!columns.empty()) {
        columns_sql.clear();
        for (auto &c : columns) {
            columns_sql += c + ",";
        }
        columns_sql.pop_back();
    }

    std::string sql;
    if (where_clause.empty()) {
        sql = "SELECT " + columns_sql + " FROM " + table_name + ";";
    } else {
        sql = "SELECT " + columns_sql + " FROM " + table_name + " WHERE " + where_clause + ";";
    }

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        debug() << "dump_table: prepare failed: " << sqlite3_errmsg(db) << "\n";
        return {};
    }

    std::vector<std::vector<std::string>> info = table_info(table_name);

    data::TypedTable table;

    auto add_column_from_info = [&](const std::string &col_name) {
        for (auto &i : info) {
            if (i[1] == col_name) {
                const std::string &t = i[2];
                if (t == "TEXT") {
                    table.newColumn({i[1], data::RECORD_TYPE::TEXT});
                } else if (t == "INTEGER") {
                    table.newColumn({i[1], data::RECORD_TYPE::INTEGER});
                } else if (t == "REAL") {
                    table.newColumn({i[1], data::RECORD_TYPE::REAL});
                } else if (t == "BLOB") {
                    table.newColumn({i[1], data::RECORD_TYPE::BLOB});
                } else {
                    table.newColumn({i[1], data::RECORD_TYPE::TEXT});
                }
                break;
            }
        }
    };

    if (columns.empty()) {
        for (auto &row : info) {
            const std::string &t = row[2];
            if (t == "TEXT") {
                table.newColumn({row[1], data::RECORD_TYPE::TEXT});
            } else if (t == "INTEGER") {
                table.newColumn({row[1], data::RECORD_TYPE::INTEGER});
            } else if (t == "REAL") {
                table.newColumn({row[1], data::RECORD_TYPE::REAL});
            } else if (t == "BLOB") {
                table.newColumn({row[1], data::RECORD_TYPE::BLOB});
            } else {
                table.newColumn({row[1], data::RECORD_TYPE::TEXT});
            }
        }
    } else {
        for (auto &c : columns) {
            // strip DISTINCT if present
            if (c.size() > 9 && strncasecmp(c.c_str(), "DISTINCT ", 9) == 0) {
                c.erase(0, 9);
            }
            add_column_from_info(c);
        }
    }

    const int col_count = sqlite3_column_count(stmt);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> row_strings;
        row_strings.reserve(col_count);

        for (int col = 0; col < col_count; ++col) {
            int col_type = sqlite3_column_type(stmt, col);
            switch (col_type) {
            case SQLITE_NULL:
                row_strings.emplace_back("");
                break;

            case SQLITE_INTEGER: {
                sqlite3_int64 v = sqlite3_column_int64(stmt, col);
                row_strings.emplace_back(std::to_string(v));
                break;
            }

            case SQLITE_FLOAT: {
                double v = sqlite3_column_double(stmt, col);
                row_strings.emplace_back(std::to_string(v));
                break;
            }

            case SQLITE_TEXT: {
                const unsigned char *txt = sqlite3_column_text(stmt, col);
                row_strings.emplace_back(txt ? (const char *)txt : "");
                break;
            }

            case SQLITE_BLOB: {
                const void *blob = sqlite3_column_blob(stmt, col);
                int blob_len = sqlite3_column_bytes(stmt, col);

                std::string blob_data(static_cast<const char *>(blob), blob_len);
                row_strings.emplace_back(blob_data);
                break;
            }

            default:
                row_strings.emplace_back("");
                break;
            }
        }

        table.insert_fromString(row_strings);
    }

    sqlite3_finalize(stmt);
    return table;
}

data::TypedTable grDB::dump_table_OLD(std::string table_name, std::vector<std::string> columns, std::string where_clause) {

    std::vector<std::vector<std::string>> results;

    std::string columns_sql = "*";

    if (columns.size() > 0) {
        columns_sql = "";
        for (auto c : columns) {
            columns_sql += c + ",";
        }
        columns_sql.pop_back();
    }

    std::string sql;

    if (where_clause == "") {
        sql = "SELECT " + columns_sql + " from " + table_name + ";";
    } else {
        sql = "SELECT " + columns_sql + " from " + table_name + " WHERE " + where_clause + ";";
    }

    sqlite3_exec(db, sql.c_str(), callback, &results, &error);

    // Remove "DISTINCT" from all the strings in columns
    for (auto &c : columns) {
        if (istarts_with(c, "DISTINCT ")) {
            c.erase(0, 9);
        }
    }

    data::TypedTable table;

    if (results.size() > 0) {

        std::vector<std::vector<std::string>> info = table_info(table_name);

        if (columns.size() == 0) {
            // Construct TypedTable based on pragma table_info results
            for (auto row : info) {
                if (row[2] == "TEXT") {
                    table.newColumn({row[1], data::RECORD_TYPE::TEXT});

                } else if (row[2] == "INTEGER") {
                    table.newColumn({row[1], data::RECORD_TYPE::INTEGER});

                } else if (row[2] == "REAL") {
                    table.newColumn({row[1], data::RECORD_TYPE::REAL});
                }
            }

        } else {

            if (results[0].size() != columns.size()) {
                std::cerr << "Error: number of columns in results does not match number of columns in columns parameter" << std::endl;
                exit(-1);
            }

            // Construct TypedTable based on columns parameter
            for (auto c : columns) {
                for (auto i : info) {
                    if (i[1] == c) {
                        if (i[2] == "TEXT") {
                            table.newColumn({i[1], data::RECORD_TYPE::TEXT});
                            break;

                        } else if (i[2] == "INTEGER") {
                            table.newColumn({i[1], data::RECORD_TYPE::INTEGER});
                            break;

                        } else if (i[2] == "REAL") {
                            table.newColumn({i[1], data::RECORD_TYPE::REAL});
                            break;

                        } else if (i[2] == "BLOB") {
                            table.newColumn({i[1], data::RECORD_TYPE::BLOB});
                            break;
                        }
                    }
                }
            }
        }

        // Insert results into TypedTable
        for (auto row : results) {
            table.insert_fromString(row);
        }
    }

    return table;
}

std::vector<grDB::field_t> grDB::select_column(std::string table_name, std::string column_name, std::string where_clause) {

    data::TypedTable table = dump_table(table_name, {column_name}, where_clause);

    if (table.getNumberColumns() > 1) {
        std::cerr << "Error in grDB::select_column: More than 1 column retrieved" << std::endl;
    }

    // Traverse table and return column
    std::vector<field_t> column;

    for (auto row : table.data()) {
        for (auto field : row) {
            column.push_back(field);
        }
    }

    return column;
}