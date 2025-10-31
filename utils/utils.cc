/* SPDX-License-Identifier: AGPL-3.0-only */
#include "utils.h"

std::array<unsigned char, 20> git_hash_object(std::filesystem::path p) {

    std::string content = read_file(p);
    size_t size = content.size();

    // git hash-object = "blob " + length + "\0" + content;
    unsigned char *git_format = new unsigned char[size + 24];
    unsigned char *ptr = git_format;

    memcpy(ptr, "blob ", 5);
    ptr += 5;

    memcpy(ptr, std::to_string(size).c_str(), std::to_string(size).size());
    ptr += std::to_string(size).size();

    *ptr = '\0';
    ptr++;

    memcpy(ptr, content.c_str(), size);
    ptr += size;

    size_t git_format_size = ptr - git_format;

    unsigned char hash[SHA_DIGEST_LENGTH]; // == 20

    SHA1(git_format, git_format_size, hash);

    delete[] git_format;

    std::array<unsigned char, 20> result;

    std::copy(hash, hash + SHA_DIGEST_LENGTH, result.begin());

    return result;
}

bool is_hex_string(const std::string &s) {

    // TODO: Improve this heuristic

    // Count the number of '0x' occurrences
    size_t count = 0;
    size_t pos = 0;
    while ((pos = s.find("0x", pos)) != std::string::npos) {
        count++;
        pos += 2;
    }

    if (count > s.length() / 8) {
        return true;
    }

    return false;
}

uint32_t HexToBytes(const std::string &hex, uint8_t *outBuf) {

    // std::vector<char> bytes;

    uint32_t ptr = 0;

    for (uint32_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = (char)strtol(byteString.c_str(), NULL, 16);
        // bytes.push_back(byte);
        outBuf[ptr] = byte;
        ptr++;
    }

    return ptr;
}

std::string hex_decode(const std::string &hex) {

    std::string bytes;

    // In the format "0x12 0x34 0x56 ..."
    for (size_t i = 0; i < hex.length();) {

        if (hex[i] == ' ') {
            i++;
            continue;
        }

        if (hex.length() - i < 4) {
            break;
        }

        std::string byte_char = hex.substr(i, 4);

        if (byte_char[0] == '0' && byte_char[1] == 'x' && isxdigit(byte_char[2]) && isxdigit(byte_char[3])) {

            std::string byteString = byte_char.substr(2, 2);
            char byte = (char)strtol(byteString.c_str(), NULL, 16);
            bytes.push_back(byte);
            i += 4;

        } else {
            i++;
        }
    }

    return bytes;
}

std::string random_password(uint32_t length) {

    // For entropy

    thread_local std::random_device rd;
    thread_local std::mt19937_64 rng{rd()};

    std::string output;

    while (output.length() < length) {

        uint64_t n = rng();

        char (&bytes)[8] = *reinterpret_cast<char (*)[8]>(&n);

        std::string tmp(bytes, 8);

        for (auto c : tmp) {
            if (isalnum(c)) {
                output += c;
            } else if (isblank(c)) {
                output += '_';
            } else if (isspace(c)) {
                output += '.';
            } else if (c == '?' || c == '!' || c == '@' || c == '#' || c == '$' || c == '%' || c == '&' || c == '*' || c == '-' || c == '+') {
                output += c;
            }
        }
    }

    return output.substr(0, length);
}

std::string base64_encode(const std::string &in) {

    std::string out;

    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

std::string base64_decode(const std::string &in) {

    std::string out;

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1)
            break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

uint64_t toint(const std::thread::id &id) {

    std::ostringstream ss;
    ss << id;
    std::string threadID = ss.str();

    uint64_t intID = std::stoull(threadID);

    return intID;
}

std::string format_duration(uint64_t duration) {

    std::chrono::nanoseconds ns = std::chrono::nanoseconds(duration);

    using namespace std::chrono;

    auto secs = duration_cast<seconds>(ns);
    ns -= duration_cast<milliseconds>(secs);

    auto mins = duration_cast<minutes>(secs);
    secs -= duration_cast<seconds>(mins);

    auto hour = duration_cast<hours>(mins);
    mins -= duration_cast<minutes>(hour);

    std::stringstream ss;
    ss << hour.count() << " hours " << mins.count() << " minutes " << secs.count() << " seconds";
    return ss.str();
}

std::string current_time_str() {

    std::string str;

    time_t now = time(0);
    tm *ltm = localtime(&now);

    std::string month = std::to_string(1 + ltm->tm_mon);
    std::string zeroed_month = std::string(2 - month.length(), '0') + month;

    std::string day = std::to_string(ltm->tm_mday);
    std::string zeroed_day = std::string(2 - day.length(), '0') + day;

    std::string hour = std::to_string(ltm->tm_hour);
    std::string zeroed_hour = std::string(2 - hour.length(), '0') + hour;

    std::string minutes = std::to_string(ltm->tm_min);
    std::string zeroed_minutes = std::string(2 - minutes.length(), '0') + minutes;

    std::string seconds = std::to_string(ltm->tm_sec);
    std::string zeroed_seconds = std::string(2 - seconds.length(), '0') + seconds;

    str = std::to_string(1900 + ltm->tm_year) + "_" + zeroed_month + "_" + zeroed_day + "__" + zeroed_hour + "_" + zeroed_minutes + "_" +
          zeroed_seconds;
    return str;
}

std::unordered_map<std::string, std::string> parse_configuration_file(std::string file_path) {

    std::unordered_map<std::string, std::string> configuration;

    std::ifstream infile(file_path);

    std::string line;
    while (std::getline(infile, line)) {

        if (line.starts_with("#") || line.empty()) {
            continue;
        }

        auto pos = line.find_first_of('=');
        if (pos == std::string::npos) {
            continue;
        }

        // std::vector<std::string> v = split(line, '=');

        configuration[line.substr(0, pos)] = line.substr(pos + 1);
    }

    return configuration;
}

std::string memory_dump(void *ptr, size_t size) {

    unsigned char *memory_addr = (unsigned char *)ptr;

    char buffer[size * 3 + 1];
    char *buf_ptr = buffer;

    for (int i = 0; i < size; i++) {
        // printf("%02X ", memory_addr[i]); fflush(stdout);
        sprintf(buf_ptr, "%02X ", memory_addr[i]);
        buf_ptr += 3;
    }
    buffer[size * 3] = '\0';

    return std::string(buffer);
}

std::string get_password_masked(const std::string &prompt) {
    std::cout << prompt;
    std::cout.flush();
    std::string pwd;

    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);
    termios raw = oldt;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    for (;;) {
        int ch = getchar();
        if (ch == '\n' || ch == '\r' || ch == EOF) {
            std::cout << "\n";
            break;
        }
        if (ch == 127 || ch == 8) { // backspace on most terms
            if (!pwd.empty()) {
                pwd.pop_back();
                std::cout << "\b \b";
            }
        } else if (ch >= 32 && ch < 127) {
            pwd.push_back(char(ch));
            std::cout << '*';
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return pwd;
}
