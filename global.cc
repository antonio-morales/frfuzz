/* SPDX-License-Identifier: AGPL-3.0-only */
#include "global.h"

FRglobal &FRglobal::init() {

    // Check if there is an environment variable called "FRFUZZ_DEBUG_LEVEL"
    const char *debug_level = getenv("FRFUZZ_DEBUG_LEVEL");
    if (debug_level != nullptr) {
        this->debug_level = std::stoi(debug_level);
    }

    if (this->debug_level > 0) {
        enable_debug(true);
        std::cout << "Debug mode enabled. Level: " << this->debug_level << std::endl;
    }

    // Try to get the current user
    struct passwd *pw = getpwuid(getuid());
    if (pw == nullptr) {
        std::cerr << "Error: Could not get current user" << std::endl;
        exit(-1);
    }

    this->CURRENT_USER = pw->pw_name;

    std::string home = getenv("HOME");

    // If we have a valid home, set the HOME_PATH variable. If not, we can try with /home/CURRENT_USER
    this->HOME_PATH = home;
    if (!std::filesystem::exists(HOME_PATH)) {
        this->HOME_PATH = "/home/" + CURRENT_USER;
        if (!std::filesystem::exists(HOME_PATH)) {
            std::cerr << "Error: Could not find home directory" << std::endl;
            exit(-1);
        }
    }

    this->FRFUZZ_PATH = this->HOME_PATH / ".frfuzz";
    create_dir(this->FRFUZZ_PATH, false);

    this->PROJECTS_PATH = this->FRFUZZ_PATH / "projects";
    create_dir(this->PROJECTS_PATH, false);

    if (!docker::check_docker_http_api()) {
        std::cerr << "Error in check_docker_http_api: Docker does not seem to be running" << std::endl;
    }

    // DOWNLOADS_PATH = USERDATA_PATH + "/Downloads";
    // TMP_PATH = USERDATA_PATH + "/Tmp";
    this->GLOBAL_DB_PATH = FRFUZZ_PATH / "global.db";

    // Try to open/create the global database
    grDB *db = new grDB();

    // std::string db_path = USERDATA_PATH + "/GRFuzz.db";
    auto error = db->open(this->GLOBAL_DB_PATH);

    if (error == grDB::ERROR::FILE_NOT_FOUND) {

        std::cout << "Global database file not found. Creating a new one..." << std::endl;
        if (!db->create(this->GLOBAL_DB_PATH)) {
            std::cerr << "Error creating database " << this->GLOBAL_DB_PATH << std::endl;
            exit(-1);
        }
    } else if (error == grDB::ERROR::WRONG_FILE_FORMAT) {

        std::cerr << "Not a valid GRFuzz database" << std::endl;
        exit(-1);
    } else if (error == grDB::ERROR::INTERNAL_ERROR) {

        std::cerr << "Error opening database " << this->GLOBAL_DB_PATH << std::endl;
        exit(-1);
    } else if (error == grDB::ERROR::OK) {

        debug() << "Database file found. Opening it..." << std::endl;
    }

    // Global database is now open
    this->global_db = db;

    // Check if there is already a user/password in the database
    data::TypedTable users_table = this->global_db->dump_table("users");
    if (users_table.empty()) {

        // If not, create a new user/password and store it in the database
        debug() << "DEBUG: The table 'users' is empty" << std::endl;
        std::cout << "No users found in the database. New user/password will be created" << std::endl;

        std::string user = "grUser";
        std::string password = random_password();

        data::TypedTable new_user_table({{"username", data::RECORD_TYPE::TEXT}, {"password", data::RECORD_TYPE::TEXT}});
        new_user_table.insert({user, password});

        // TODO: Password must be hashed

        this->global_db->insert("users", new_user_table, "username");

        std::cout << "User: " << user << std::endl;
        std::cout << "Password: " << password << std::endl;
    } else {
        debug() << "DEBUG: The table 'users' was found in the database" << std::endl;
    }

    // Check if there is already a 'secrets' table in the database
    if (!this->global_db->table_exists("secrets")) {

        debug() << "DEBUG: The table 'secrets' does not exist in the database" << std::endl;
        debug() << "New secrets table will be created" << std::endl;

        data::TypedTable new_secrets_table({{"name", data::RECORD_TYPE::TEXT}, {"value", data::RECORD_TYPE::BLOB}});
        new_secrets_table.insert({"OPENAI_API_KEY", "0"});

        this->global_db->insert("secrets", new_secrets_table, "name");
    } else {
        debug() << "DEBUG: The table 'secrets' was found in the database" << std::endl;
    }

    return *this;
}
