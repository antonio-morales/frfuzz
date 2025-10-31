/* SPDX-License-Identifier: AGPL-3.0-only */
#include "filesys.h"

std::string get_mime_info(const std::filesystem::path &path, std::string info) {

    // Retrieve the MIME type of a file with libmagic

    magic_t magic_cookie;

    if (info == "type") {
        magic_cookie = magic_open(MAGIC_MIME_TYPE);

    } else if (info == "encoding") {
        magic_cookie = magic_open(MAGIC_MIME_ENCODING);

    } else if (info == "") {
        magic_cookie = magic_open(MAGIC_NONE);

    } else {
        std::cout << "Error in get_mime_info: Invalid info parameter" << std::endl;
        return "";
    }

    if (magic_cookie == NULL) {
        std::cout << "unable to initialize magic library" << std::endl;
        return "";
    }

    if (magic_load(magic_cookie, NULL) != 0) {
        std::cout << "cannot load magic database - " << magic_error(magic_cookie) << std::endl;
        magic_close(magic_cookie);
        return "";
    }

    const char *mime = magic_file(magic_cookie, path.c_str());
    if (mime == NULL) {
        std::cout << "cannot get magic file description - " << magic_error(magic_cookie) << std::endl;
        magic_close(magic_cookie);
        return "";
    }

    std::string result(mime);

    // std::cout << "MIME type of test.txt is " << mime << std::endl;
    magic_close(magic_cookie);

    return result;
}

bool is_executable_file(const std::filesystem::path &path) {

    std::string mime_encoding = get_mime_info(path, "encoding");

    if (mime_encoding == "binary") {

        // std::cout << path << " : " << mime_encoding << std::endl;

        std::string mime_type = get_mime_info(path, "type");
        if (mime_type == "application/x-pie-executable") {

            // std::cout << path << " : " << mime_type << std::endl;

            // std::string mime_info = get_mime_info(path);
            // std::cout << path << " : " << mime_info << std::endl;
            // std::cout << std::endl;

            return true;
        }
    }

    return false;
}

folder_snapshot::folder_snapshot(std::filesystem::path folder_path) {

    // Get a list of all the generated files
    // std::vector<std::filesystem::path> generated_files;

    // recursive search in build folder
    for (const auto &entry : std::filesystem::recursive_directory_iterator(folder_path)) {

        if (entry.is_regular_file()) {

            // std::cout << entry.path().string() << std::endl;

            // std::cout << std::filesystem::relative(entry.path(), folder_path).string() << std::endl;

            // std::cout << std::endl;

            file_hash[std::filesystem::relative(entry.path(), folder_path)] = git_hash_object(entry.path());

        } else if (entry.is_directory()) {

            file_hash[std::filesystem::relative(entry.path(), folder_path)] = {};
        }
    }
}

std::vector<std::filesystem::path> folder_snapshot::diff(const std::filesystem::path &folder_path, bool recursive) {

    std::vector<std::filesystem::path> new_files;

    if (recursive) {

        // recursive search in build folder
        for (const auto &entry : std::filesystem::recursive_directory_iterator(folder_path)) {

            if (entry.is_regular_file()) {

                std::string relative_path = std::filesystem::relative(entry.path(), folder_path);

                if (!file_hash.contains(relative_path)) {
                    // New file
                    new_files.push_back(entry.path()); // Full path

                } else {

                    std::array<unsigned char, 20> hash = git_hash_object(entry.path());
                    if (hash != file_hash[relative_path]) {
                        // File changed
                        new_files.push_back(entry.path()); // Full path
                    }
                }
            }
        }

    } else {

        for (const auto &entry : std::filesystem::directory_iterator(folder_path)) {

            if (entry.is_regular_file() || entry.is_directory()) {

                std::string relative_path = std::filesystem::relative(entry.path(), folder_path);

                if (!file_hash.contains(relative_path)) {
                    // New file
                    new_files.push_back(entry.path()); // Full path

                } else {

                    if (entry.is_directory()) {
                        continue;
                    }
                    std::array<unsigned char, 20> hash = git_hash_object(entry.path());
                    if (hash != file_hash[relative_path]) {
                        new_files.push_back(entry.path()); // Full path
                    }
                }
            }
        }
    }

    return new_files;
}

char *read_fd(int fd, size_t &size) {

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        return nullptr;
    }

    size = st.st_size;
    char *buf = new char[size];
    if (read(fd, buf, size) == -1) {
        perror("read");
        return nullptr;
    }

    return buf;
}

std::string read_fd(int fd) {

    size_t size;

    char *buf = read_fd(fd, size);
    if (buf == nullptr) {
        return "";
    }

    std::string str(buf, size);
    delete[] buf;
    return str;
}

char *read_file(const std::string path, size_t &size) {

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("open");
        return nullptr;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        return nullptr;
    }

    size = st.st_size;
    char *buf = new char[size];
    if (read(fd, buf, size) == -1) {
        perror("read");
        return nullptr;
    }

    close(fd);
    return buf;
}

std::string old_read_file(const std::string filename) {

    size_t size = 0;
    char *buf = read_file(filename.c_str(), size);
    if (buf == nullptr) {
        return "";
    }

    std::string str(buf, size);
    delete[] buf;
    return str;
}

std::string read_file(const std::filesystem::path file_path) {

    if (!std::filesystem::exists(file_path)) {
        std::cout << "Error in read_file: file does not exist" << std::endl;
        return "";
    }

    uintmax_t filesize = std::filesystem::file_size(file_path);

    std::fstream fs;

    fs.open(file_path, std::fstream::in | std::fstream::binary);
    if (fs.fail()) {
        std::cout << "Error in read_file: could not open file" << std::endl;
        return "";
    }

    char buf[filesize];

    fs.read(buf, filesize);
    if (!fs) {
        std::cout << "Error in read_file: read failed" << std::endl;
        return "";
    }

    fs.close();

    return std::string(buf, filesize);
}

std::string read_file(const std::filesystem::path file_path, int &error_code) {

    std::string str = read_file(file_path);

    if (str.empty()) {
        error_code = -1;
    }

    if (!str.empty()) {
        error_code = 0; // No error
    }

    return str;
}

// Read file as a vector of bytes
std::vector<uint8_t> read_file_bytes(std::filesystem::path file_path) {

    uintmax_t filesize = std::filesystem::file_size(file_path);

    std::fstream fs;

    fs.open(file_path, std::fstream::in | std::fstream::binary);
    if (fs.fail()) {
        std::cout << "Error in read_file_bytes: could not open file" << std::endl;
        return {};
    }

    uint8_t buffer[filesize];

    fs.read((char *)&buffer, filesize);
    if (!fs) {
        std::cout << "Error in read_file_bytes: read failed" << std::endl;
        return {};
    }

    return std::vector<uint8_t>(&buffer[0], &buffer[filesize]);
}

std::vector<uint8_t> read_file_bytes(std::filesystem::path file_path, std::error_code &error) {

    std::vector<uint8_t> bytes = read_file_bytes(file_path);

    if (bytes.empty()) {
        error = std::make_error_code(std::errc::io_error);
    }

    if (!bytes.empty()) {
        error = std::error_code(); // No error
    }

    return bytes;
}

bool write_file(const std::string path, std::string file_contents) {

    std::ofstream outfile;

    outfile.open(path, std::ios::out | std::ios::binary | std::ios::trunc);

    if (outfile.is_open()) {
        /* ok, proceed with output */

        outfile.write(file_contents.data(), file_contents.size());

        outfile.close();

        if (outfile.fail()) {
            criticalError();
        }

    } else {
        criticalError();
    }

    return true;
}

bool create_dir(std::filesystem::path path, bool recursive) {

    // Check if directory does not exist
    if (!std::filesystem::is_directory(path)) {

        if (recursive) {
            if (!std::filesystem::create_directories(path)) {
                return false;
            }

        } else {
            if (!std::filesystem::create_directory(path)) {
                return false;
            }
        }
    }

    return true;
}

/*
bool create_dir_recursively(std::string path){

    //Check if directory does not exist
    if(!std::filesystem::is_directory(path)){

        if(!std::filesystem::create_directories(path)){
            return false;
        }
    }

    return true;
}
*/

bool mkdir_if_not_exist(std::string dir_path) {

    bool folder_created = false;

    struct stat st = {0};

    if (stat(dir_path.c_str(), &st) == -1) {
        mkdir(dir_path.c_str(), 0700);
        folder_created = true;
    }

    return folder_created;
}

// C++ 23
bool contains_subrange() { return true; }

// Check if path is inside folder
bool inside_directory(std::string path, std::string folder) {

    std::filesystem::path normalized_local_path = std::filesystem::weakly_canonical(path);

    std::filesystem::path normalized_folder_path = std::filesystem::weakly_canonical(folder);

    auto it = std::search(normalized_local_path.begin(), normalized_local_path.end(), normalized_folder_path.begin(), normalized_folder_path.end());

    if (it == normalized_local_path.end()) {
        return false;
    }

    return true;
}

std::filesystem::path file_numbering(std::filesystem::path filepath) {

    if (!std::filesystem::exists(filepath)) {
        return filepath;
    }

    std::string parent_path = filepath.parent_path().string();

    std::filesystem::path filename = filepath.filename();

    std::string ext = filename.extension().string();
    std::filesystem::path name = filename.stem();

    if (name.extension() == ".tar") {
        name = name.stem();
        ext = ".tar" + ext;
    }

    std::string path;
    uint32_t i;

    for (i = 0; i <= UINT32_MAX; i++) {

        path = parent_path + "/" + name.string() + " (" + std::to_string(i) + ")" + ext;

        if (!std::filesystem::exists(path)) {
            break;
        }
    }

    return path;

    // return std::filesystem::path(parent_path + "/" + name.string() + " (" + std::to_string(i) + ext);

    /*
        std::string path = filepath.parent_path().string() + "/" + name +  filename.extension().string();

        //Check if file has a number at the end
        if(!name.empty() && name.back() == ')' ){

            name.pop_back();

            size_t pos = name.find_last_of('(');
            if(pos == std::string::npos){
                return "";
            }

            //Get the number
            int number;
            try{
                number = std::stoi(name.substr(pos + 1));

            }catch (const std::invalid_argument & e) {
                std::cout << e.what() << "\n";
                return "";
            }
            catch (const std::out_of_range & e) {
                std::cout << e.what() << "\n";
                return "";
            }

            //Increment the number
            number++;

            //Remove the number from the filename
            name = name.string().substr(0, name.string().find_last_of("."));

            //Add the new number to the filename
            name += "." + std::to_string(number);

            //Add the extension
            name += filename.extension().string();

            //Return the new filename
            return name.string();
        }
    */
}

std::vector<std::filesystem::path> files_to_disk(std::vector<file_t> files, std::string local_folder, bool auto_numbering) {

    std::vector<std::filesystem::path> saved_files = {};

    std::filesystem::path normalized_local_path = std::filesystem::weakly_canonical(local_folder);

    for (auto f : files) {

        std::filesystem::path path(local_folder + "/" + f.path);

        if (!inside_directory(path, local_folder)) {
            std::cout << "Error in files_to_disk: Path traversal bug detected" << std::endl;
            continue;
        }

        if (!create_dir(path.parent_path().string(), true)) {
            std::cout << "Error in files_to_disk: Could not create directory: " << path.parent_path().string() << std::endl;
            continue;
        }

        if (auto_numbering) {
            path = file_numbering(path);
        }

        if (!write_file(path, f.content)) {
            std::cout << "Error in files_to_disk: Could not write file: " << path.string() << std::endl;
        } else {
            saved_files.push_back(path);
        }
    }

    return saved_files;
}

std::filesystem::path get_cwd() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return std::string(cwd);
    } else {
        exit(EXIT_FAILURE);
    }
}

std::string get_hostname() {
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    return std::string(hostname);
}

std::filesystem::path get_home_dir() {

    struct passwd *pw = getpwuid(getuid());
    return std::string(pw->pw_dir);
}

std::time_t last_modified(std::filesystem::path path) {

    struct stat attr;
    stat(path.c_str(), &attr);
    return attr.st_mtime;
}

std::filesystem::path unique_folder_name(const std::filesystem::path &parent_folder, std::string folder_prefix, std::string type) {

    std::filesystem::path output_path;

    std::string output_folder;

    if (type == "ExtendedDescription") {

        // Append current date
        output_folder = folder_prefix + "__" + current_time_str();

        // Append hostname
        output_folder += "__" + get_hostname();

        // Append memory limit

        output_path = parent_folder / output_folder;
        size_t n = 2;
        while (std::filesystem::exists(output_path)) {
            output_folder = "output_" + std::to_string(n);
            output_path = parent_folder / output_folder;
            n++;
        }

    } else {
        std::cerr << "Error in unique_folder_name: unknown type" << std::endl;
        exit(EXIT_FAILURE);
    }

    return output_path;
}

bool copy_files(const std::filesystem::path &input_folder, const std::filesystem::path &output_folder, std::string extension, size_t max_length,
                bool reverse) {

    for (auto &p : std::filesystem::recursive_directory_iterator(input_folder)) {

        if (p.is_regular_file()) {

            if (extension != "") {

                std::string ext = p.path().extension().string();

                // non-case sensitive comparison
                to_lower(ext);

                if (extension != ext) {
                    // std::cout << "Skipping " << p.path() << std::endl;
                    continue;
                }
            }

            std::filesystem::path new_path = output_folder / p.path().filename();
            if (std::filesystem::exists(new_path)) {
                // Add a number to the filename
                int i = 1;
                while (std::filesystem::exists(new_path)) {
                    new_path = output_folder / (p.path().stem().string() + "_" + std::to_string(i) + p.path().extension().string());
                    i++;
                }
            }

            // std::cout << "Copying " << p.path() << " to " << new_path << std::endl;

            // Copy only the first max_bytes bytes
            std::ifstream file(p.path(), std::ios::binary);
            std::ofstream new_file(new_path, std::ios::binary);

            char buffer[max_length];

            if (reverse) {

                size_t file_size = std::filesystem::file_size(p.path());

                // Seek to the end of the file
                file.seekg(file_size - max_length, std::ios::beg);
            }

            file.read(buffer, max_length);
            file.close();

            new_file.write(buffer, file.gcount());
            new_file.close();

            // std::filesystem::copy_file(p, new_path, std::filesystem::copy_options::overwrite_existing);
        }
    }
}
