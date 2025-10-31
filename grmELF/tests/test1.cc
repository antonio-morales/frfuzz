/* SPDX-License-Identifier: AGPL-3.0-only */
// Compare readelf output with DWARF parser output

#include <filesystem>
#include <string>

#include "../dwarf.h"
#include "../elf.h"

#include "grmUtils/process.h"

// Remove all leading and trailing spaces from the input and compress all other spaces to a single character
std::string trim_all(const std::string &input) {

    std::string out;

    for (auto it = input.begin(); it != input.end(); ++it) {
        // std::cout << *it << std::endl;
        // std::cout << "pausa" << std::endl;

        if (std::isblank(*it)) {
            out += ' ';
            while (it != input.end() && std::isblank(*it)) {
                ++it;
            }
            --it;
        } else {
            out += *it;
        }
    }

    return out;
}

bool test1(std::filesystem::path elf_path) {

    if (!is_executable_file(elf_path)) {
        std::cout << "Error: " << elf_path << " is not an executable file" << std::endl;
        exit(1);
    }

    std::cout << "Testing " << elf_path << std::endl;

    std::string command = "readelf --debug-dump=info " + elf_path.string();

    std::string readelf_output = run(command);
    readelf_output.erase(0, 40); // Remove "Contents of the .debug_info section:\n\n "

    readelf_output = trim_all(readelf_output);

    ELF elf_file;

    elf_file.open(elf_path);

    Elf64_Addr addr = elf_file.get_symbol_addr("main");

    DWARF dwarf(&elf_file);

    std::stringstream ss;

    ss << dwarf;

    std::string output = ss.str();

    output = trim_all(output);

    write_file("/tmp/readelf_output.txt", readelf_output);

    write_file("/tmp/output.txt", output);
}

int main(int argc, char *argv[]) {

    std::filesystem::path root_path = "/home/...";

    for (auto d : std::filesystem::directory_iterator(root_path / "tests/elf_samples")) {

        if (d.is_regular_file()) {

            if (test1(d.path()) == false) {
                std::cout << "Test1 failed for " << d.path() << std::endl;
                exit(1);
            }
        }
    }
}
