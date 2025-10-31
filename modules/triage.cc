/* SPDX-License-Identifier: AGPL-3.0-only */
#include "triage.h"

std::string triage_asan_summary(const TRIAGE_RESULT &results, std::vector<std::filesystem::path> crashes_folders, size_t total_crashes) {

    auto &bugs = results.triage_asan_result->sym_bugs;
    auto &aborted = results.triage_asan_result->aborted;
    auto &unknown = results.triage_asan_result->unknown;

    std::stringstream file;

    file << "<!DOCTYPE html>\n";
    file << "<html>\n";
    file << "<head>\n";

    file << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.6.0/jquery.min.js\"></script>\n";
    file << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery.tablesorter/2.31.3/js/jquery.tablesorter.min.js\"></script>\n";

    file << "<style>\n";
    file << "table {\n";
    file << "  font-family: arial, sans-serif;\n";
    file << "  border-collapse: collapse;\n";
    file << "  width: 100%;\n";
    file << "}\n";
    file << "\n";
    file << "td, th {\n";
    file << "  border: 1px solid #dddddd;\n";
    file << "  text-align: left;\n";
    file << "  padding: 8px;\n";
    file << "}\n";
    file << "\n";
    file << "tr:nth-child(even) {\n";
    file << "  background-color: #dddddd;\n";
    file << "}\n";
    file << "</style>\n";
    file << "</head>\n";
    file << "<body>\n";
    file << "\n";
    file << "<h2>Summary</h2>\n";
    file << "\n";

    // List all the crashes folders
    file << "<h3>Crashes folders</h3>\n";
    file << "<ul>\n";
    for (auto &crash_folder : crashes_folders) {
        file << "  <li>" << crash_folder << "</li>\n";
    }
    file << "</ul>\n";

    file << "<p>Total crashes: " << total_crashes << "</p>\n";
    file << "<p>Unique bugs: " << bugs.size() << "</p>\n";

    file << "<table class=\"sortable\">\n";
    file << "<thead>\n";
    file << "  <tr>\n";
    file << "    <th>Sanitizer</th>\n";
    file << "    <th>Bug Type</th>\n";
    file << "    <th>Call stack</th>\n";
    file << "    <th>File</th>\n";
    file << "    <th>Line</th>\n";
    file << "    <th>Number of crashes</th>\n";
    file << "    <th>Crashes</th>\n";
    file << "    <th>Details</th>\n";
    file << "  </tr>\n";
    file << "</thead>\n";

    file << "<tbody>\n";
    for (auto &bug : bugs) {

        std::vector<FR_CRASH> crashes = bug.second;

        file << "  <tr>\n";

        if (bug.first.sanitizer == SANITIZER::ASAN) {
            file << "    <td>AddressSanitizer</td>\n";
        } else if (bug.first.sanitizer == SANITIZER::MSAN) {
            file << "    <td>MemorySanitizer</td>\n";
        } else if (bug.first.sanitizer == SANITIZER::UBSAN) {
            file << "    <td>UndefinedBehaviorSanitizer</td>\n";
        } else if (bug.first.sanitizer == SANITIZER::TSAN) {
            file << "    <td>ThreadSanitizer</td>\n";
        } else if (bug.first.sanitizer == SANITIZER::CFISAN) {
            file << "    <td>Control Flow Integrity Sanitizer</td>\n";
        } else {
            file << "    <td>Unknown</td>\n";
        }

        if (bug.first.type == BUG_TYPE::HEAP_BUFFER_OVERFLOW) {
            file << "    <td>Heap Buffer Overflow";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::HEAP_BUFFER_OVERFLOW_READ) {
            file << "    <td>Heap Buffer Overflow (READ)";

            // Get the 3 FR_CRASH with the most oob_bytes
            std::sort(crashes.begin(), crashes.end(), [](FR_CRASH &a, FR_CRASH &b) { return a.oob_bytes > b.oob_bytes; });
            file << " [up to " << crashes[0].oob_bytes << " bytes]";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::HEAP_BUFFER_OVERFLOW_WRITE) {
            file << "    <td>Heap Buffer Overflow (WRITE)";

            std::sort(crashes.begin(), crashes.end(), [](FR_CRASH &a, FR_CRASH &b) { return a.oob_bytes > b.oob_bytes; });
            file << " [up to " << crashes[0].oob_bytes << " bytes]";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::STACK_BUFFER_OVERFLOW) {
            file << "    <td>Stack Buffer Overflow";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::STACK_BUFFER_OVERFLOW_READ) {
            file << "    <td>Stack Buffer Overflow (READ)";

            // Get the 3 FR_CRASH with the most oob_bytes
            std::sort(crashes.begin(), crashes.end(), [](FR_CRASH &a, FR_CRASH &b) { return a.oob_bytes > b.oob_bytes; });
            file << " [up to " << crashes[0].oob_bytes << " bytes]";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::STACK_BUFFER_OVERFLOW_WRITE) {
            file << "    <td>Stack Buffer Overflow (WRITE)";

            std::sort(crashes.begin(), crashes.end(), [](FR_CRASH &a, FR_CRASH &b) { return a.oob_bytes > b.oob_bytes; });
            file << " [up to " << crashes[0].oob_bytes << " bytes]";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::SEGV_READ) {
            file << "    <td>Segmentation Fault (READ)</td>\n";
        } else if (bug.first.type == BUG_TYPE::SEGV_WRITE) {
            file << "    <td>Segmentation Fault (WRITE)</td>\n";
        } else if (bug.first.type == BUG_TYPE::SEGV) {
            file << "    <td>Segmentation Fault</td>\n";

        } else if (bug.first.type == BUG_TYPE::ALLOCATION_OVERFLOW) {
            file << "    <td>Allocation Overflow</td>\n";
        } else if (bug.first.type == BUG_TYPE::FPE) {
            file << "    <td>Floating Point Exception</td>\n";
        } else if (bug.first.type == BUG_TYPE::OOM) {
            file << "    <td>Out of Memory</td>\n";

        } else if (bug.first.type == BUG_TYPE::UAF) {
            file << "    <td>Use After Free</td>\n";

        } else if (bug.first.type == BUG_TYPE::UAF_READ) {
            file << "    <td>Use After Free (READ)";

            std::sort(crashes.begin(), crashes.end(), [](FR_CRASH &a, FR_CRASH &b) { return a.oob_bytes > b.oob_bytes; });
            file << " [up to " << crashes[0].oob_bytes << " bytes]";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::UAF_WRITE) {
            file << "    <td>Use After Free (WRITE)";

            std::sort(crashes.begin(), crashes.end(), [](FR_CRASH &a, FR_CRASH &b) { return a.oob_bytes > b.oob_bytes; });
            file << " [up to " << crashes[0].oob_bytes << " bytes]";
            file << "</td>\n";

        } else if (bug.first.type == BUG_TYPE::GLOBAL_BUFFER_OVERFLOW) {
            file << "    <td>Global Buffer Overflow</td>\n";
        } else if (bug.first.type == BUG_TYPE::STACK_OVERFLOW) {
            file << "    <td>Stack Overflow</td>\n";

        } else if (bug.first.type == BUG_TYPE::NEGATIVE_SIZE) {
            file << "    <td>Negative Size Parameter</td>\n";

        } else if (bug.first.type == BUG_TYPE::INVALID_FREE) {
            file << "    <td>Invalid Free</td>\n";

        } else if (bug.first.type == BUG_TYPE::UNKNOWN) {
            file << "    <td>Unknown crash</td>\n";
        } else {
            file << "    <td>Unknown</td>\n";
        }

        file << "    <td>" << bug.first.function << "</td>\n";
        file << "    <td>" << bug.first.file << "</td>\n";
        file << "    <td>" << bug.first.line << "</td>\n";
        file << "    <td>" << bug.second.size() << "</td>\n";

        const size_t max_crashes = 16;
        size_t c;

        file << "    <td>";
        c = max_crashes;
        for (auto &crash : crashes) {
            if (c == 0) {
                break;
            }
            file << "<a href=\"" << crash.crash_path.string() << "\">" << crash.crash_path.filename().string() << "</a> <br> \n";
            c--;
        }
        file << "    </td>\n";

        file << "    <td>";
        c = max_crashes;
        for (auto &crash : crashes) {
            if (c == 0) {
                break;
            }
            file << "<details>" << crash.description << "</details> <br> \n";
            c--;
        }
        file << "    </td>\n";

        file << "  </tr>\n";
    }

    file << "</tbody>\n";
    file << "</table>\n";
    file << "\n";

    file << "<h3>Aborted inputs</h3>\n";
    file << "<ul>\n";
    for (auto &abort : aborted) {
        file << "  <li>" << abort.crash_path << "</li><details>" << abort.description << "</details>\n";
    }
    file << "</ul>\n";

    file << "<h3>Unknown crashes</h3>\n";
    file << "<ul>\n";

    const size_t MAX_UNKNOWN = 512;

    size_t cnt = MAX_UNKNOWN;

    for (auto &u : unknown) {
        file << "  <li>" << u.crash_path << "</li><details>" << u.description << "</details>\n";
        cnt--;
        if (cnt == 0) {
            break;
        }
    }
    file << "</ul>\n";

    file << "<script>\n";
    file << "$(document).ready(function() {\n";
    file << "  $(\".sortable\").tablesorter();\n";
    file << "});\n";
    file << "</script>\n";

    file << "</body>\n";
    file << "</html>\n";

    return file.str();
}

std::string triage_ubsan_summary(const TRIAGE_RESULT &results, std::vector<std::filesystem::path> crashes_folders, size_t total_crashes) {}

std::string triage_gdb_summary(const TRIAGE_RESULT &results, std::vector<std::filesystem::path> crashes_folders, size_t total_crashes) {

    auto &bugs = results.triage_gdb_result->bugs;

    std::stringstream file;

    file << "<!DOCTYPE html>\n";
    file << "<html>\n";
    file << "<head>\n";

    file << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.6.0/jquery.min.js\"></script>\n";
    file << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery.tablesorter/2.31.3/js/jquery.tablesorter.min.js\"></script>\n";

    file << "<style>\n";
    file << "table {\n";
    file << "  font-family: arial, sans-serif;\n";
    file << "  border-collapse: collapse;\n";
    file << "  width: 100%;\n";
    file << "}\n";
    file << "\n";
    file << "td, th {\n";
    file << "  border: 1px solid #dddddd;\n";
    file << "  text-align: left;\n";
    file << "  padding: 8px;\n";
    file << "}\n";
    file << "\n";
    file << "tr:nth-child(even) {\n";
    file << "  background-color: #dddddd;\n";
    file << "}\n";
    file << "</style>\n";
    file << "</head>\n";
    file << "<body>\n";
    file << "\n";
    file << "<h2>Summary</h2>\n";
    file << "\n";

    // List all the crashes folders
    file << "<h3>Crashes folders</h3>\n";
    file << "<ul>\n";
    for (auto &crash_folder : crashes_folders) {
        file << "  <li>" << crash_folder << "</li>\n";
    }
    file << "</ul>\n";

    file << "<p>Total crashes: " << total_crashes << "</p>\n";
    file << "<p>Unique bugs: " << bugs.size() << "</p>\n";

    file << "<table class=\"sortable\">\n";
    file << "<thead>\n";
    file << "  <tr>\n";
    file << "    <th>Bug Type</th>\n";
    file << "    <th>Vulnerable Function</th>\n";
    file << "    <th>Arguments</th>\n";
    file << "    <th>File</th>\n";
    file << "    <th>Line</th>\n";
    file << "    <th>Number of crashes</th>\n";
    file << "    <th>Crashes</th>\n";
    file << "    <th>Details</th>\n";
    file << "  </tr>\n";
    file << "</thead>\n";

    file << "<tbody>\n";
    for (auto &bug : bugs) {

        std::vector<FR_CRASH> &crashes = bug.second;

        file << "  <tr>\n";
        file << "    <td>Segmentation Fault</td>\n";

        file << "    <td>" << bug.first.gdb_func << "</td>\n";
        file << "    <td>" << bug.first.gdb_arg << "</td>\n";
        file << "    <td>" << bug.first.gdb_file << "</td>\n";
        file << "    <td>" << bug.first.gdb_line << "</td>\n";

        file << "    <td>" << bug.second.size() << "</td>\n";

        const size_t max_crashes = 4;
        size_t c;

        file << "    <td>";
        c = max_crashes;
        for (auto &crash : crashes) {
            if (c == 0) {
                break;
            }
            file << "<a href=\"" << crash.crash_path.string() << "\">" << crash.crash_path.filename().string() << "</a> <br> \n";
            c--;
        }
        file << "    </td>\n";

        file << "    <td>";
        c = max_crashes;
        for (auto &crash : crashes) {
            if (c == 0) {
                break;
            }
            file << "<details>" << crash.description << "</details> <br> \n";
            c--;
        }
        file << "    </td>\n";

        file << "  </tr>\n";
    }

    file << "</tbody>\n";
    file << "</table>\n";
    file << "\n";

    file << "<script>\n";
    file << "$(document).ready(function() {\n";
    file << "  $(\".sortable\").tablesorter();\n";
    file << "});\n";
    file << "</script>\n";

    file << "</body>\n";
    file << "</html>\n";

    return file.str();
}

std::string triage_malloc_summary(const TRIAGE_RESULT &results, std::vector<std::filesystem::path> crashes_folders, size_t total_crashes) {

    auto &detected = results.triage_malloc_result->detected;

    std::stringstream file;

    file << "<!DOCTYPE html>\n";
    file << "<html>\n";
    file << "<head>\n";

    file << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.6.0/jquery.min.js\"></script>\n";
    file << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery.tablesorter/2.31.3/js/jquery.tablesorter.min.js\"></script>\n";

    file << "<style>\n";
    file << "table {\n";
    file << "  font-family: arial, sans-serif;\n";
    file << "  border-collapse: collapse;\n";
    file << "  width: 100%;\n";
    file << "}\n";
    file << "\n";
    file << "td, th {\n";
    file << "  border: 1px solid #dddddd;\n";
    file << "  text-align: left;\n";
    file << "  padding: 8px;\n";
    file << "}\n";
    file << "\n";
    file << "tr:nth-child(even) {\n";
    file << "  background-color: #dddddd;\n";
    file << "}\n";
    file << "</style>\n";
    file << "</head>\n";
    file << "<body>\n";
    file << "\n";
    file << "<h2>Summary</h2>\n";
    file << "\n";

    // List all the crashes folders
    file << "<h3>Crashes folders</h3>\n";
    file << "<ul>\n";
    for (auto &crash_folder : crashes_folders) {
        file << "  <li>" << crash_folder.string() << "</li>\n";
    }
    file << "</ul>\n";

    file << "<p>Total detections: " << detected.size() << "</p>\n";

    file << "<h3>Detected inputs</h3>\n";

    file << "<table class=\"sortable\">\n";
    file << "<thead>\n";
    file << "  <tr>\n";
    file << "    <th>Crash Path</th>\n";
    file << "    <th>Description</th>\n";
    file << "    <th>Details</th>\n";
    file << "  </tr>\n";
    file << "</thead>\n";

    file << "<tbody>\n";

    for (auto &crash : detected) {

        file << "  <tr>\n";

        file << "    <td>";
        file << "<a href=\"" << crash.crash_path.string() << "\">" << crash.crash_path.filename().string() << "</a> <br> \n";
        file << "    </td>\n";

        file << "    <td>";
        file << crash.malloc_msg << "<br> \n";
        file << "    </td>\n";

        file << "    <td>";
        file << "<details>" << crash.description << "</details> <br> \n";
        file << "    </td>\n";

        file << "  </tr>\n";
    }

    file << "</tbody>\n";
    file << "</table>\n";
    file << "\n";

    file << "<script>\n";
    file << "$(document).ready(function() {\n";
    file << "  $(\".sortable\").tablesorter();\n";
    file << "});\n";
    file << "</script>\n";

    file << "</body>\n";
    file << "</html>\n";

    return file.str();
}

std::string triage_summary(TRIAGE_RESULT &results, const std::vector<std::filesystem::path> &crashes_folders, size_t total_crashes,
                           std::string parser) {

    // const std::unordered_map<FR_BUG, std::vector<FR_CRASH>> bugs, std::vector<std::filesystem::path> crashes_folders, size_t total_crashes,
    // std::vector<FR_CRASH> aborted, std::vector<FR_CRASH> unknown, std::string parser) {

    if (parser == "ASAN") {
        return triage_asan_summary(results, crashes_folders, total_crashes);

    } else if (parser == "UBSAN") {
        return triage_ubsan_summary(results, crashes_folders, total_crashes);

    } else if (parser == "GDB") {
        return triage_gdb_summary(results, crashes_folders, total_crashes);

    } else if (parser == "MALLOC") {
        return triage_malloc_summary(results, crashes_folders, total_crashes);

    } else {
        std::cerr << "Error: unknown parser: " << parser << std::endl;
        exit(EXIT_FAILURE);
    }
}

//<binary, address>
std::tuple<std::filesystem::path, uint64_t> parse_stack_trace_line_NOSYM(std::string line) {

    std::filesystem::path file = "";
    uint64_t address = 0;

    size_t pos, pos2;

    if ((pos = line.find(" (")) != std::string::npos) {

        pos += 2;

        if ((pos2 = line.find(") ", pos)) != std::string::npos) {

            // Find last '+' character before ')'
            size_t addr_pos;
            if ((addr_pos = line.rfind('+', pos2)) != std::string::npos) {

                std::string address_str = line.substr(addr_pos + 1, pos2 - addr_pos - 1);

                // Check if the address is an hex number
                if (address_str.starts_with("0x") != std::string::npos) {
                    address = std::stoull(address_str, nullptr, 16);
                } else {
                    std::cerr << "Error: address is not an hex number: " << address_str << std::endl;
                    exit(EXIT_FAILURE);
                }

                std::string file_str = line.substr(pos, addr_pos - pos);
                file = std::filesystem::path(file_str);
            }

            // line = line.substr(pos, pos2 - pos);
        }
    }

    return std::make_tuple(file, address);
}

//<function, file, line_number>
std::tuple<std::string, std::filesystem::path, size_t> parse_stack_trace_line(std::string line) {

    std::string function = "";
    std::filesystem::path file = "";
    size_t line_number = 0;

    size_t pos;

    if ((pos = line.find(" in ")) != std::string::npos) {

        // Find next space after " in "
        pos += 4;

        // We need to find the next space or '('
        size_t aux1 = line.find(' ', pos);
        size_t aux2 = line.find('(', pos);

        size_t pos_aux = std::min(aux1, aux2);

        function = line.substr(pos, pos_aux - pos);

        pos += function.size() + 1;
        line = line.substr(pos);

        // Find : in the rest of the string
        if ((pos = line.find(':')) != std::string::npos) {

            file = line.substr(0, pos);

            // Only check for line number if the file is a C or C++ file (to avoid searching for line numbers in external libraries)
            if (file.extension().string() == ".c" || file.extension().string() == ".cpp" || file.extension().string() == ".cc") {

                line = line.substr(pos + 1, line.find(':', pos + 1) - pos - 1);

                // Check if the line is a number
                if (is_number(line)) {
                    line_number = std::stoi(line);
                } else {
                    std::cout << "Error: line is not a number: " << line << std::endl;
                    // std::cout << output << std::endl;
                    exit(1);
                }
            }

        } else {
            file = line;
        }

    } else {

        // Function name is not present in the stack trace

        // Get the number of the stack trace, finding the # character
        if ((pos = line.find("#")) != std::string::npos) {

            pos += 1;

            if ((pos = line.find("0x", pos)) != std::string::npos) {

                if ((pos = line.find(" ")) != std::string::npos) {

                    pos += 1;

                    // Find next character different from space
                    while (pos < line.size() && line[pos] == ' ') {
                        pos += 1;
                    }

                    // Find : in the rest of the string
                    if ((pos = line.find(':')) != std::string::npos) {

                        file = line.substr(0, pos);

                        // Only check for line number if the file is a C or C++ file (to avoid searching for line numbers in external libraries)
                        if (file.extension().string() == ".c" || file.extension().string() == ".cpp" || file.extension().string() == ".cc") {

                            line = line.substr(pos + 1, line.find(':', pos + 1) - pos - 1);

                            // Check if the line is a number
                            if (is_number(line)) {
                                line_number = std::stoi(line);
                            } else {
                                std::cout << "Error: line is not a number: " << line << std::endl;
                                // std::cout << output << std::endl;
                                exit(1);
                            }
                        }

                    } else {
                        file = line;
                    }

                } else {
                    std::cout << "Look like there is no file in the stack trace: " << line << std::endl;
                    exit(1);
                }

            } else {
                std::cout << "Error: could not find '0x' in line: " << line << std::endl;
                exit(1);
            }

        } else {
            std::cout << "Error: could not find '#' in line: " << line << std::endl;
            exit(1);
        }
    }

    return std::make_tuple(function, file, line_number);
}

std::optional<std::tuple<FR_NOSYM_BUG, FR_CRASH>> parse_sanitizer_output_NOSYM(std::string output, const std::filesystem::path triage_folder,
                                                                               const std::filesystem::path binary_folder) {

    FR_NOSYM_BUG bug;

    FR_CRASH crash;

    size_t pos = 0;

    // Search the output for the sanitizer used

    if ((pos = output.find("ERROR: AddressSanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::ASAN;
        pos += 25;
    } else if ((pos = output.find("ERROR: MemorySanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::MSAN;
        pos += 24;
    } else if ((pos = output.find("ERROR: UndefinedBehaviorSanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::UBSAN;
        pos += 35;
    } else if ((pos = output.find("ERROR: ThreadSanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::TSAN;
        pos += 24;
    } else if ((pos = output.find("ERROR: Control Flow Integrity Sanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::CFISAN;
        pos += 40;
    } else {
        return std::nullopt;
    }

    // String from pos to the end of the file
    output = output.substr(pos);
    crash.description = output;
    pos = 0;

    // Look if the following text from pos is a bug type

    //(pos = output.find(stack_trace_num, pos)) != std::string::npos

    if (output.starts_with("heap-buffer-overflow")) {
        if ((pos = output.find("READ of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::HEAP_BUFFER_OVERFLOW_READ;
            pos += 13;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else if ((pos = output.find("WRITE of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::HEAP_BUFFER_OVERFLOW_WRITE;
            pos += 14;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else {
            bug.type = BUG_TYPE::HEAP_BUFFER_OVERFLOW;
        }
        // pos += 20;

    } else if (output.starts_with("stack-buffer-overflow")) {
        if ((pos = output.find("READ of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::STACK_BUFFER_OVERFLOW_READ;
            pos += 13;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }

        } else if ((pos = output.find("WRITE of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::STACK_BUFFER_OVERFLOW_WRITE;
            pos += 14;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else {
            bug.type = BUG_TYPE::STACK_BUFFER_OVERFLOW;
        }

    } else if (output.starts_with("SEGV")) {
        if (output.find("caused by a READ") != std::string::npos) {
            bug.type = BUG_TYPE::SEGV_READ;
        } else if (output.find("caused by a WRITE") != std::string::npos) {
            bug.type = BUG_TYPE::SEGV_WRITE;
        } else {
            bug.type = BUG_TYPE::SEGV;
        }
        pos += 4;

    } else if (output.starts_with("attempting free on address which was not malloc")) {

        bug.type = BUG_TYPE::INVALID_FREE;
        pos += 47;

    } else if (output.starts_with("requested allocation size")) {
        bug.type = BUG_TYPE::ALLOCATION_OVERFLOW;
        pos += 25;
    } else if (output.starts_with("FPE")) {
        bug.type = BUG_TYPE::FPE;
        pos += 3;
    } else if (output.starts_with("out of memory")) {
        bug.type = BUG_TYPE::OOM;
        pos += 13;

    } else if (output.starts_with("heap-use-after-free")) {
        if ((pos = output.find("READ of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::UAF_READ;
            pos += 13;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else if ((pos = output.find("WRITE of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::UAF_WRITE;
            pos += 14;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else {
            bug.type = BUG_TYPE::UAF;
        }
        // pos += 19;

    } else if (output.starts_with("global-buffer-overflow")) {
        bug.type = BUG_TYPE::GLOBAL_BUFFER_OVERFLOW;
        pos += 22;

    } else if (output.starts_with("stack-overflow")) {
        bug.type = BUG_TYPE::STACK_OVERFLOW;
        pos += 14;

    } else if (output.starts_with("negative-size-param")) {
        bug.type = BUG_TYPE::NEGATIVE_SIZE;
        pos += 19;

    } else if (output.starts_with("unknown-crash")) {
        bug.type = BUG_TYPE::UNKNOWN;
        pos += 13;

    } else {
        std::cerr << output << std::endl;
        std::cerr << "Unknown bug type" << std::endl;
        exit(1);
    }

    // Get function
    // if (bug.type == BUG_TYPE::ALLOCATION_OVERFLOW || bug.type == BUG_TYPE::OOM || bug.type == BUG_TYPE::UNKNOWN) {

    // stack_trace_num = "    #1";
    //}

    size_t func_num = 0;

    std::string stack_trace_num = "    #" + std::to_string(func_num);

    if ((pos = output.find(stack_trace_num, pos)) != std::string::npos) {

        // pos += 6;

        std::tuple<std::string, std::filesystem::path, size_t> p;

        std::tuple<std::filesystem::path, uint64_t> q;

        std::filesystem::path saved_file = "";
        size_t saved_line = 0;
        uint64_t saved_address = 0;

        do {

            // pos = output.find('\n', pos) + 1;

            // String from pos to the end of the line
            std::string curr_line = output.substr(pos, output.find('\n', pos) - pos);

            pos += curr_line.size() + 1;

            // if (!rem_line.starts_with("    #")) {
            // break;
            //}

            q = parse_stack_trace_line_NOSYM(curr_line);

            bug.stack_trace[func_num] = q;

            func_num += 1;
            stack_trace_num = "    #" + std::to_string(func_num);

        } while ((pos = output.find(stack_trace_num, pos)) != std::string::npos && func_num < MAX_STACK_DEPTH);

        // bug.function = std::get<0>(p);

        // bug.file = saved_file;
        // bug.file = std::filesystem::relative(bug.file, triage_folder); // Remove the parent path from the file

        // bug.address = saved_address;

    } else {

        // No stack trace found

        // std::cout << output << std::endl;
        // exit(1);
        // return std::nullopt;
    }

    // Return the optional with the bug
    return std::make_tuple(bug, crash);
}

std::optional<std::tuple<FR_BUG, FR_CRASH>> parse_sanitizer_output(std::string output, const std::filesystem::path triage_folder,
                                                                   const std::filesystem::path binary_folder) {

    FR_BUG bug;

    FR_CRASH crash;

    size_t pos = 0;

    // Search the output for the sanitizer used

    if ((pos = output.find("ERROR: AddressSanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::ASAN;
        pos += 25;
    } else if ((pos = output.find("ERROR: MemorySanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::MSAN;
        pos += 24;
    } else if ((pos = output.find("ERROR: UndefinedBehaviorSanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::UBSAN;
        pos += 35;
    } else if ((pos = output.find("ERROR: ThreadSanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::TSAN;
        pos += 24;
    } else if ((pos = output.find("ERROR: Control Flow Integrity Sanitizer: ")) != std::string::npos) {
        bug.sanitizer = SANITIZER::CFISAN;
        pos += 40;
    } else {
        return std::nullopt;
    }

    // String from pos to the end of the file
    output = output.substr(pos);
    crash.description = output;
    pos = 0;

    // Look if the following text from pos is a bug type

    //(pos = output.find(stack_trace_num, pos)) != std::string::npos

    if (output.starts_with("heap-buffer-overflow")) {
        if ((pos = output.find("READ of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::HEAP_BUFFER_OVERFLOW_READ;
            pos += 13;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else if ((pos = output.find("WRITE of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::HEAP_BUFFER_OVERFLOW_WRITE;
            pos += 14;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else {
            bug.type = BUG_TYPE::HEAP_BUFFER_OVERFLOW;
        }
        // pos += 20;

    } else if (output.starts_with("stack-buffer-overflow")) {
        if ((pos = output.find("READ of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::STACK_BUFFER_OVERFLOW_READ;
            pos += 13;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }

        } else if ((pos = output.find("WRITE of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::STACK_BUFFER_OVERFLOW_WRITE;
            pos += 14;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else {
            bug.type = BUG_TYPE::STACK_BUFFER_OVERFLOW;
        }

    } else if (output.starts_with("SEGV")) {
        if (output.find("caused by a READ") != std::string::npos) {
            bug.type = BUG_TYPE::SEGV_READ;
        } else if (output.find("caused by a WRITE") != std::string::npos) {
            bug.type = BUG_TYPE::SEGV_WRITE;
        } else {
            bug.type = BUG_TYPE::SEGV;
        }
        pos += 4;

    } else if (output.starts_with("attempting free on address which was not malloc")) {

        bug.type = BUG_TYPE::INVALID_FREE;
        pos += 47;

    } else if (output.starts_with("requested allocation size")) {
        bug.type = BUG_TYPE::ALLOCATION_OVERFLOW;
        pos += 25;
    } else if (output.starts_with("FPE")) {
        bug.type = BUG_TYPE::FPE;
        pos += 3;
    } else if (output.starts_with("out of memory")) {
        bug.type = BUG_TYPE::OOM;
        pos += 13;

    } else if (output.starts_with("heap-use-after-free")) {
        if ((pos = output.find("READ of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::UAF_READ;
            pos += 13;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else if ((pos = output.find("WRITE of size ")) != std::string::npos) {
            bug.type = BUG_TYPE::UAF_WRITE;
            pos += 14;
            std::string bytes = output.substr(pos, output.find(" ", pos) - pos);
            if (is_number(bytes)) {
                crash.oob_bytes = std::stoll(bytes);
            } else {
                std::cout << output << std::endl;
                return std::nullopt;
            }
        } else {
            bug.type = BUG_TYPE::UAF;
        }
        // pos += 19;

    } else if (output.starts_with("global-buffer-overflow")) {
        bug.type = BUG_TYPE::GLOBAL_BUFFER_OVERFLOW;
        pos += 22;

    } else if (output.starts_with("stack-overflow")) {
        bug.type = BUG_TYPE::STACK_OVERFLOW;
        pos += 14;

    } else if (output.starts_with("negative-size-param")) {
        bug.type = BUG_TYPE::NEGATIVE_SIZE;
        pos += 19;

    } else if (output.starts_with("unknown-crash")) {
        bug.type = BUG_TYPE::UNKNOWN;
        pos += 13;

    } else {
        std::cerr << output << std::endl;
        std::cerr << "Unknown bug type" << std::endl;
        exit(1);
    }

    // Get function
    // if (bug.type == BUG_TYPE::ALLOCATION_OVERFLOW || bug.type == BUG_TYPE::OOM || bug.type == BUG_TYPE::UNKNOWN) {

    // stack_trace_num = "    #1";
    //}

    const size_t MAX_STACK_DEPTH = 5;

    size_t func_num = 0;

    std::string stack_trace_num = "    #" + std::to_string(func_num);

    if ((pos = output.find(stack_trace_num, pos)) != std::string::npos) {

        // pos += 6;

        std::tuple<std::string, std::filesystem::path, size_t> p;

        std::tuple<std::filesystem::path, uint64_t> q;

        std::filesystem::path saved_file = "";
        size_t saved_line = 0;
        uint64_t saved_address = 0;

        do {

            // pos = output.find('\n', pos) + 1;

            // String from pos to the end of the line
            std::string curr_line = output.substr(pos, output.find('\n', pos) - pos);

            pos += curr_line.size() + 1;

            // if (!rem_line.starts_with("    #")) {
            // break;
            //}

            p = parse_stack_trace_line(curr_line);

            if (std::get<1>(p).string().starts_with("../")) {
                // Modify p [1]
                std::get<1>(p) = binary_folder / std::get<1>(p);
                // std::cout << "Modified path: " << std::get<1>(p) << std::endl;

                // Normalize path
                std::get<1>(p) = std::filesystem::weakly_canonical(std::get<1>(p));
                // std::cout << "Normalized path: " << std::get<1>(p) << std::endl;
            }

            if (func_num == 0) {
                saved_file = std::get<1>(p);
                saved_line = std::get<2>(p);

            } else {
                bug.function = " > " + bug.function;
            }

            bug.function = std::get<0>(p) + bug.function;

            func_num += 1;
            stack_trace_num = "    #" + std::to_string(func_num);

        } while ((pos = output.find(stack_trace_num, pos)) != std::string::npos && func_num < MAX_STACK_DEPTH);

        // bug.function = std::get<0>(p);

        bug.file = saved_file;
        bug.file = std::filesystem::relative(bug.file, triage_folder); // Remove the parent path from the file

        bug.line = saved_line;

    } else {

        // No stack trace found

        // std::cout << output << std::endl;
        // exit(1);
        // return std::nullopt;
    }

    // Return the optional with the bug
    return std::make_tuple(bug, crash);
}

FR_BUG symbolize(FR_NOSYM_BUG bug) {

    FR_BUG sym_bug;

    sym_bug.sanitizer = bug.sanitizer;
    sym_bug.type = bug.type;

    for (int i = MAX_STACK_DEPTH - 1; i >= 0; i--) {

        std::string filepath = std::get<0>(bug.stack_trace[i]);
        uint64_t address = std::get<1>(bug.stack_trace[i]);

        if (filepath == "" || address == 0) {
            continue;
        }

        // Convert address to hex
        std::string address_str = "0x" + std::format("{:x}", address);

        std::string cmd = "addr2line --exe=" + filepath + " --demangle --functions " + address_str;

        std::string output = run(cmd);

        std::istringstream iss(output);

        std::string l;

        do {
            std::getline(iss, l);
        } while (l.find("Dwarf Error") != std::string::npos || l.find("DWARF error") != std::string::npos);

        std::string function = l;

        size_t aux = function.find('(');
        if (aux != std::string::npos) {
            function = function.substr(0, aux);
        }

        std::string file = "";
        std::string line = "";

        if (function == "??") {
            std::filesystem::path obj_file = filepath;
            function = obj_file.filename().string() + "+" + address_str;

        } else {

            do {
                std::getline(iss, l);
            } while (l.find("Dwarf Error") != std::string::npos || l.find("DWARF error") != std::string::npos);

            // Look for ':' in the file line
            size_t pos;
            if ((pos = l.find(':')) != std::string::npos) {

                file = l.substr(0, pos);

                if (file == "??") {
                    file = "";
                    line = "0";

                } else {

                    size_t pos2;
                    if ((pos2 = l.find(':', pos + 1)) != std::string::npos) {
                        line = l.substr(pos + 1, pos2 - pos - 1);
                    } else {
                        line = l.substr(pos + 1);
                    }

                    if (line == "?") {
                        line = "0";

                    } else if (!is_number(line)) {
                        std::cerr << "Error: line is not a number: " << line << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }

            } else {
                file = l;
            }
        }

        sym_bug.function += function;

        if (i == 0) {
            sym_bug.file = file;
            sym_bug.line = std::stoi(line);

        } else {
            sym_bug.function += " > ";
        }
    }

    return sym_bug;
}

void symbolize_results(TRIAGE_RESULT &results, std::filesystem::path triage_folder) {

    auto &bugs = results.triage_asan_result->bugs;
    auto &sym_bugs = results.triage_asan_result->sym_bugs;

    // Check if "addr2line" is installed
    std::string checkcmd = "addr2line --version";

    std::string output = run(checkcmd);

    if (output.find("addr2line") == std::string::npos) {
        std::cerr << "Error: addr2line is not installed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Symbolize the crashes
    std::cout << std::endl;
    std::cout << "Symbolizing the results..." << std::endl;

    for (auto &bug : bugs) {

        FR_BUG sym_bug = symbolize(bug.first);

        sym_bug.file = std::filesystem::relative(sym_bug.file, triage_folder); // Remove the parent path from the file

        if (sym_bugs.count(sym_bug) > 0) {
            sym_bugs[sym_bug].insert(sym_bugs[sym_bug].end(), bug.second.begin(), bug.second.end());

        } else {
            sym_bugs.insert({sym_bug, {bug.second}});
        }
    }
}

void triage_asan(std::string cmd, std::filesystem::path crash_path, std::filesystem::path triage_folder, const std::filesystem::path binary_folder,
                 TRIAGE_RESULT &triage_results, size_t repeat) {

    if (triage_results.triage_asan_result == nullptr) {
        triage_results.triage_asan_result = new TRIAGE_ASAN_RESULT();
    }

    TRIAGE_ASAN_RESULT *results = triage_results.triage_asan_result;

    std::string output;

    bool found = false;

    for (int r = 0; r < repeat && !found; r++) {

        // if (r > 0) {
        //  std::cout << "\t Repeating..." << std::endl;
        //}

        // output = run(cmd + crashes[i].string());

        output = run(cmd, 60000); // 60 seconds timeout

        // std::cout << output << std::endl;

        std::optional<std::tuple<FR_NOSYM_BUG, FR_CRASH>> result = parse_sanitizer_output_NOSYM(output, triage_folder, binary_folder);

        if (result.has_value()) {

            // Bug found

            FR_NOSYM_BUG bug = std::get<0>(result.value());
            FR_CRASH fr_crash = std::get<1>(result.value());
            fr_crash.crash_path = crash_path;

            // Check if the bug is already in the list
            if (results->bugs.count(bug) > 0) {

                results->bugs[bug].push_back(fr_crash);

            } else {

                results->bugs.insert({bug, {fr_crash}});
            }

            found = true;

        } else {

            // No bug found by parse_sanitizer_output

            // Check if output is an aborted
            if (output.find("Aborted") != std::string::npos) {

                FR_CRASH abort;

                abort.crash_path = crash_path;
                abort.description = output;

                results->aborted.push_back(abort);

                found = true;

            } else {
                // std::cout << "\t No bug found at repeat " << r << std::endl;
            }
        }
    }

    if (!found) {

        FR_CRASH u;
        u.crash_path = crash_path;
        u.description = output;

        results->unknown.push_back(u);

        // std::cerr << "Failed to triage crash: " << crashes[i] << std::endl;
        // std::cout << output << std::endl;
        //  exit(EXIT_FAILURE);
    }

    // std::cout << std::endl;
}

std::tuple<std::string, std::string, std::string, size_t> parse_gdb_line(std::string line) {

    std::string function = "";
    std::string argument = "";
    std::string filename = "";
    size_t linenumber = 0;

    size_t pos;

    if ((pos = line.find(" ")) != std::string::npos) {

        function = line.substr(0, pos);

        line = line.substr(pos + 1);

        pos = 0;

        if (function.starts_with("0x") && line.starts_with("in ")) {

            pos += 3;

            // Find next whitespace
            if ((pos = line.find(" ", pos)) != std::string::npos) {

                function = line.substr(3, pos - 3);

                line = line.substr(pos + 1);
            }

            // std::cout << line << std::endl;
        }

        // std::cout << "Function: " << function << std::endl;

        if ((pos = line.find(" at ")) != std::string::npos || (pos = line.find(" from ")) != std::string::npos) {

            if (pos > 0) {
                argument = line.substr(0, pos);
                // std::cout << "Argument: " << argument << std::endl;
            }

            filename = line.substr(pos + 4);

            // Get now the line number

            if ((pos = filename.find(":")) != std::string::npos) {

                std::string line_number = filename.substr(pos + 1);

                filename = filename.substr(0, pos);

                linenumber = std::stoi(line_number);

                // std::cout << "File: " << filename << std::endl;

                // std::cout << "Line: " << linenumber << std::endl;
            }
        }
    }

    return std::make_tuple(function, argument, filename, linenumber);
}

void triage_gdb(std::string cmd, std::filesystem::path crash_path, const std::filesystem::path triage_folder,
                const std::filesystem::path binary_folder, TRIAGE_RESULT &triage_results, size_t repeat) {

    if (triage_results.triage_gdb_result == nullptr) {
        triage_results.triage_gdb_result = new TRIAGE_GDB_RESULT();
    }

    TRIAGE_GDB_RESULT *results = triage_results.triage_gdb_result;

    std::string output;

    bool found = false;

    std::string command = "gdb";

    command += " --batch"; // Batch mode

    command += " -ex 'run'";

    command += " -ex 'quit'"; // Continue the execution

    command += " --args " + cmd; // Load the binary"

    for (int r = 0; r < repeat && !found; r++) {

        output = run(command, 15000); // 15 seconds timeout

        // std::cout << output << std::endl;

        size_t pos;

        if ((pos = output.find("SIGSEGV")) != std::string::npos || (pos = output.find("SIGABRT")) != std::string::npos) {

            found = true;

            output = output.substr(pos);

            std::stringstream ss(output);

            // Get the first line
            std::string line;

            if (std::getline(ss, line)) {

                // std::cout << line << std::endl;

                if (std::getline(ss, line)) {

                    // std::cout << line << std::endl;

                    while (line.starts_with("[Switching to Thread")) {

                        // std::cout << "Skipping line..." << std::endl;

                        if (!std::getline(ss, line)) {
                            break;
                        }
                    }

                    // std::cout << line << std::endl;

                    // Parse the line to get function name, file and line

                    auto tuple = parse_gdb_line(line);

                    GDB_BUG bug;
                    bug.gdb_func = std::get<0>(tuple);
                    bug.gdb_arg = std::get<1>(tuple);
                    bug.gdb_file = std::get<2>(tuple);

                    if (bug.gdb_file.starts_with("../")) {

                        bug.gdb_file = binary_folder / bug.gdb_file;
                        bug.gdb_file = std::filesystem::weakly_canonical(bug.gdb_file);
                    }

                    bug.gdb_line = std::get<3>(tuple);

                    if (bug.gdb_func == "" || bug.gdb_file == "") {
                        std::cerr << "Error: Failed to parse gdb output" << std::endl;
                        std::cout << output << std::endl;
                        // exit(EXIT_FAILURE);
                    }

                    FR_CRASH crash;
                    crash.crash_path = crash_path;
                    crash.description = output;

                    // Check if the bug is already in the list
                    if (results->bugs.count(bug) > 0) {

                        results->bugs[bug].push_back(crash);

                    } else {

                        results->bugs.insert({bug, {crash}});
                    }
                }
            }
        }
    }
}

void triage_malloc(std::string cmd, std::filesystem::path crash_path, const std::filesystem::path triage_folder,
                   const std::filesystem::path binary_folder, TRIAGE_RESULT &triage_results, size_t repeat) {

    if (triage_results.triage_malloc_result == nullptr) {
        triage_results.triage_malloc_result = new TRIAGE_MALLOC_RESULT();
    }

    TRIAGE_MALLOC_RESULT *results = triage_results.triage_malloc_result;

    std::string output;

    bool found = false;

    // for (int r = 0; r < repeat && !found; r++) {

    output = run(cmd, 15000); // 15 seconds timeout

    // std::cout << output << std::endl;

    std::string substr[] = {

        "break adjusted to free malloc space",

        "corrupted double-linked list",
        "corrupted size vs. prev_size",

        "double free or corruption",

        "free(): corrupted unsorted chunks",
        "free(): double free detected in tcache",
        "free(): invalid next size (fast)",
        "free(): invalid next size (normal)",
        "free(): invalid pointer",
        "free(): invalid size",
        "free(): too many chunks detected in tcache",

        "_int_memalign(): unaligned chunk detected",

        "invalid chunk size",
        "invalid fastbin entry",

        "malloc(): corrupted unsorted chunks",
        "malloc(): corrupted top size",
        "malloc(): invalid next->prev_inuse",
        "malloc(): invalid next size",
        "malloc(): invalid size",
        "malloc(): largebin double linked list corrupted",
        "malloc(): memory corruption",
        "malloc(): mismatching next->prev_size",
        "malloc(): smallbin double linked list corrupted",
        "malloc: top chunk is corrupt",
        "malloc(): unaligned fastbin chunk detected",
        "malloc(): unaligned tcache chunk detected",
        "malloc(): unsorted double linked list corrupted",

        "malloc_check_get_size: memory corruption",

        "malloc_consolidate(): invalid chunk size",

        "munmap_chunk(): invalid pointer",
        "mremap_chunk(): invalid pointer",

        "realloc(): invalid old size",
        "realloc(): invalid next size",
        "realloc(): invalid pointer",

        "unaligned fastbin chunk detected",
        "unaligned tcache chunk detected",

        "Fatal glibc error: malloc.c",

        "glibc error"

    };

    for (auto &sstr : substr) {

        if (output.find(sstr) != std::string::npos) {

            // std::cout << std::endl;
            // std::cout << output << std::endl;
            // std::cout << std::endl;

            FR_CRASH crash;

            crash.crash_path = crash_path;
            crash.description = output;
            crash.malloc_msg = sstr;

            results->detected.push_back(crash);

            break;
        }
    }
}

void triage_thread(size_t thread_id, const std::vector<std::filesystem::path> &crashes, std::string cmd_split1, std::string cmd_split2,
                   const std::filesystem::path triage_folder, const std::filesystem::path binary_folder, TRIAGE_RESULT &results, size_t repeat,
                   std::string parser_str) {

    enum PARSER { ASAN, UBSAN, GDB, MALLOC } parser;

    if (parser_str == "ASAN") {
        parser = PARSER::ASAN;
    } else if (parser_str == "UBSAN") {
        parser = PARSER::UBSAN;
    } else if (parser_str == "GDB") {
        parser = PARSER::GDB;
    } else if (parser_str == "MALLOC") {
        parser = PARSER::MALLOC;
    } else {
        std::cerr << "Error: Unknown parser" << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t num_elements = crashes.size();

    for (int i = 0; i < num_elements; i++) {

        if (thread_id == 0 && i % 10 == 0) {
            std::cout << "Current run: " << i + 1 << " / " << num_elements << std::endl;
        }

        std::string cmd = cmd_split1 + " " + bash_escape(crashes[i].string()) + cmd_split2;

        switch (parser) {

        case PARSER::ASAN:
            triage_asan(cmd, crashes[i], triage_folder, binary_folder, results, repeat);
            break;

        case PARSER::UBSAN:

            break;

        case PARSER::GDB:
            triage_gdb(cmd, crashes[i], triage_folder, binary_folder, results, repeat);
            break;

        case PARSER::MALLOC:
            triage_malloc(cmd, crashes[i], triage_folder, binary_folder, results, repeat);
            break;
        }
    }
}

void merge_triage_results(TRIAGE_RESULT &merged_results, const std::vector<TRIAGE_RESULT> &triage_results, std::string parser) {

    // TRIAGE_RESULT merged_results;

    size_t num_threads = triage_results.size();

    // Merge the outputs of the threads

    if (parser == "ASAN") {

        if (merged_results.triage_asan_result == nullptr) {
            merged_results.triage_asan_result = new TRIAGE_ASAN_RESULT();
        }

        for (size_t i = 0; i < num_threads; ++i) {

            for (auto &it : triage_results[i].triage_asan_result->bugs) {

                FR_NOSYM_BUG bug = std::get<0>(it);

                std::vector<FR_CRASH> fr_crashes = std::get<1>(it);

                auto &bugs = merged_results.triage_asan_result->bugs;

                if (bugs.count(bug) > 0) {
                    bugs[bug].insert(bugs[bug].end(), fr_crashes.begin(), fr_crashes.end());

                } else {
                    bugs.insert({bug, {fr_crashes}});
                }
            }

            auto &aborted = triage_results[i].triage_asan_result->aborted;
            auto &unknown = triage_results[i].triage_asan_result->unknown;

            merged_results.triage_asan_result->aborted.insert(merged_results.triage_asan_result->aborted.end(), aborted.begin(), aborted.end());
            merged_results.triage_asan_result->unknown.insert(merged_results.triage_asan_result->unknown.end(), unknown.begin(), unknown.end());
        }

    } else if (parser == "UBSAN") {

    } else if (parser == "GDB") {

        if (merged_results.triage_gdb_result == nullptr) {
            merged_results.triage_gdb_result = new TRIAGE_GDB_RESULT();
        }

        for (size_t i = 0; i < num_threads; ++i) {

            for (auto &it : triage_results[i].triage_gdb_result->bugs) {

                GDB_BUG bug = std::get<0>(it);

                std::vector<FR_CRASH> fr_crashes = std::get<1>(it);

                auto &bugs = merged_results.triage_gdb_result->bugs;

                if (bugs.count(bug) > 0) {
                    bugs[bug].insert(bugs[bug].end(), fr_crashes.begin(), fr_crashes.end());

                } else {
                    bugs.insert({bug, {fr_crashes}});
                }
            }
        }

    } else if (parser == "MALLOC") {

        if (merged_results.triage_malloc_result == nullptr) {
            merged_results.triage_malloc_result = new TRIAGE_MALLOC_RESULT();
        }

        for (size_t i = 0; i < num_threads; ++i) {

            for (FR_CRASH &crash : triage_results[i].triage_malloc_result->detected) {

                merged_results.triage_malloc_result->detected.push_back(crash);
            }
        }
    }

    // return merged_results;
}

// cmd, triage_folder, binary_folder, results, repeat);

void triage(std::string parser, std::vector<std::filesystem::path> crashes_folders, size_t repeat, const FRglobal &ctx) {

    std::filesystem::path triage_folder;

    if (parser == "ASAN") {
        triage_folder = "__ASAN_NOOPT";
    } else if (parser == "UBSAN") {
        triage_folder = "__UBSAN";
    } else if (parser == "GDB") {
        triage_folder = "__COV";
    } else if (parser == "MALLOC") {
        triage_folder = "__COV";
    }

    if (parser == "ASAN") {
        std::cout << " /*/*/* PARSER = ASAN *\\*\\*\\" << std::endl;
    } else if (parser == "UBSAN") {
        std::cout << " /*/*/* PARSER = UBSAN *\\*\\*\\" << std::endl;
    } else if (parser == "GDB") {
        std::cout << " /*/*/* PARSER = GDB *\\*\\*\\" << std::endl;
    } else if (parser == "MALLOC") {
        std::cout << " /*/*/* PARSER = MALLOC *\\*\\*\\" << std::endl;
    }
    std::cout << std::endl;

    std::filesystem::path binary_path = triage_folder / ctx.campaign->binary_rel_path;

    // TODO: Check if the binary is executable
    if (!std::filesystem::exists(binary_path) || !std::filesystem::is_regular_file(binary_path)) {
        std::cerr << "Binary " << binary_path << " does not exist or is not executable" << std::endl;
        exit(EXIT_FAILURE);
    }

    binary_path = std::filesystem::canonical(binary_path);

    std::cout << "- Binary path: " << binary_path << std::endl;
    std::cout << std::endl;

    std::string cmd = binary_path.string() + " " + ctx.campaign->binary_args;

    // std::unordered_map<FR_NOSYM_BUG, std::vector<FR_CRASH>> bugs;

    TRIAGE_RESULT results;

    std::vector<FR_CRASH> aborted;
    std::vector<FR_CRASH> unknown;

    // std::vector<FR_CRASH> dumper_results;

    size_t num_aborted = 0;

    size_t total_crashes = 0;

    auto begin = std::chrono::high_resolution_clock::now();

    for (auto &folder : crashes_folders) {

        std::cout << "Processing folder " << folder << " ..." << std::endl;
        std::cout << std::endl;

        std::vector<std::filesystem::path> read_crashes = AFL_get_crashes(folder);

        // Check that numThreads is greater than 0
        if (ctx.numThreads == 0) {
            std::cerr << "Error: numThreads must be greater than 0" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::vector<std::vector<std::filesystem::path>> crashes(ctx.numThreads);

        // Fill the array with the crashes from config.crashes
        size_t i = 0;
        for (auto &crash : read_crashes) {
            crashes[i].push_back(crash);
            i = (i + 1) % ctx.numThreads;
        }

        // std::vector<TRIAGE_RESULT> outputs(ctx.numThreads);
        // std::vector<DUMPER_RESULT> dumper_outputs(ctx.numThreads);

        std::string cmd_split1 = cmd;
        std::string cmd_split2 = "";

        size_t pos;
        if ((pos = cmd.find(" @@")) != std::string::npos) {
            cmd_split1 = cmd.substr(0, pos);
            cmd_split2 = cmd.substr(pos + 3);
        }

        if (putenv("ASAN_OPTIONS=symbolize=0") != 0) {
            std::cerr << "Error: could not set symbolize=0" << std::endl;
            exit(1);
        }

        std::vector<std::thread> threads;

        std::vector<TRIAGE_RESULT> triage_results(ctx.numThreads);

        for (size_t i = 0; i < ctx.numThreads; ++i) {

            threads.push_back(std::thread(triage_thread, i, std::cref(crashes[i]), cmd_split1, cmd_split2, triage_folder, binary_path.parent_path(),
                                          std::ref(triage_results[i]), repeat, parser));
        }

        for (auto &th : threads) {
            th.join();
        }

        /*
        if (parser == "MALLOC") {

            for (size_t i = 0; i < ctx.numThreads; ++i) {

                threads.push_back(std::thread(dumper_thread, i, std::cref(crashes[i]), cmd_split1, cmd_split2, triage_folder,
        binary_path.parent_path(), std::ref(dumper_outputs[i]), repeat));
            }

        } else {

            for (size_t i = 0; i < ctx.numThreads; ++i) {

                threads.push_back(std::thread(triage_thread, i, std::cref(crashes[i]), cmd_split1, cmd_split2, triage_folder,
        binary_path.parent_path(), std::ref(outputs[i]), repeat));
            }
        }
        */

        merge_triage_results(results, triage_results, parser);

        total_crashes += read_crashes.size();

        std::cout << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();

    num_aborted = aborted.size();

    // std::unordered_map<FR_BUG, std::vector<FR_CRASH>> sym_bugs;

    if (parser == "ASAN") {

        // Now it's time to symbolize the results
        symbolize_results(results, triage_folder);
    }

    // Display the summary

    std::cout << std::endl;
    std::cout << "Total time: " << std::dec << seconds << "secs" << std::endl;
    std::cout << std::endl;

    std::cout << "SUMMARY:" << std::endl;

    if (parser == "ASAN") {

        std::cout << "Total crashes: " << total_crashes << std::endl;

        std::cout << "Unique bugs found: " << results.triage_asan_result->sym_bugs.size() << std::endl;

        std::cout << std::endl;

        size_t bugged_files = 0;

        for (auto &bug : results.triage_asan_result->sym_bugs) {
            bugged_files += bug.second.size();
        }

        std::cout << "Total bugged files: " << bugged_files << std::endl;

        std::cout << "Aborted: " << num_aborted << std::endl;

        std::cout << "Bugs + Aborted = " << bugged_files + num_aborted << std::endl;

    } else if (parser == "GDB") {

        std::cout << "Total crashes: " << total_crashes << std::endl;

        std::cout << "Total detections: " << results.triage_gdb_result->bugs.size() << std::endl;

    } else if (parser == "MALLOC") {

        std::cout << "Total detections: " << results.triage_malloc_result->detected.size() << std::endl;
    }

    // Write an HTML summary
    std::filesystem::path summary_path = crashes_folders[0].parent_path();

    if (parser == "ASAN") {
        summary_path /= "summary__asan.html";
    } else if (parser == "UBSAN") {
        summary_path /= "summary__ubsan.html";
    } else if (parser == "GDB") {
        summary_path /= "summary__gdb.html";
    } else if (parser == "MALLOC") {
        summary_path /= "summary__malloc.html";
    }
    if (std::filesystem::exists(summary_path)) {
        summary_path = summary_path.string().erase(summary_path.string().size() - 5, 5);
        summary_path +=
            "_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        summary_path += ".html";
    }

    std::ofstream file;

    file.open(summary_path);

    std::string html_summary;

    html_summary = triage_summary(results, crashes_folders, total_crashes, parser);

    //}

    file << html_summary;

    file.close();

    std::cout << std::endl;

    if (parser == "ASAN") {
        std::cout << "- ASAN summary written to " << summary_path << std::endl;

    } else if (parser == "UBSAN") {
        std::cout << "- UBSAN summary written to " << summary_path << std::endl;

    } else if (parser == "GDB") {
        std::cout << "- GDB summary written to " << summary_path << std::endl;

    } else if (parser == "MALLOC") {
        std::cout << "- MALLOC summary written to " << summary_path << std::endl;
    }

    std::cout << std::endl;
}