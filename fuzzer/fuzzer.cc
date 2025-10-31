/* SPDX-License-Identifier: AGPL-3.0-only */
#include "fuzzer.h"

fuzzer::fuzzer() {

    // TODO: We have pointers from objects that could be died

    fuzzerPool::add(this);
}

fuzzer::~fuzzer() { fuzzerPool::remove(id); }

void fuzzer_kill() {

    const size_t delay_ms = 15;

    // Search for all afl processes reading /proc

    std::filesystem::path cpath = std::filesystem::current_path();

    std::vector<int> afl_pids;
    int grconsole_pid = -1;

    for (auto &p : std::filesystem::directory_iterator("/proc")) {

        if (!p.is_directory())
            continue;

        std::string pid = p.path().filename().string();

        if (std::all_of(pid.begin(), pid.end(), ::isdigit)) {

            std::ifstream file(p.path() / "cmdline");

            std::string line;

            std::getline(file, line);

            // Search for afl-fuzz instances or "grconsole monitor" instances
            if (line.starts_with("afl-fuzz")) {
                if (line.find(cpath) != std::string::npos) {
                    afl_pids.push_back(std::stoi(pid));
                }
            }

            if (line.starts_with("grconsole monitor")) {
                grconsole_pid = std::stoi(pid);
            }
        }
    }

    std::cout << "Found " << afl_pids.size() << " afl processes" << std::endl;

    // Get ppids of bash processes
    std::vector<int> bash_ppids;

    for (auto &pid : afl_pids) {

        std::ifstream file("/proc/" + std::to_string(pid) + "/stat");

        std::string line;

        std::getline(file, line);

        std::stringstream ss(line);

        std::string token;

        for (int i = 0; i < 4; i++) {
            std::getline(ss, token, ' ');
        }

        bash_ppids.push_back(std::stoi(token));
    }

    std::cout << "Stoping all afl processes..." << std::endl;

    for (auto &pid : afl_pids) {
        std::cout << "Sending SIGINT to " << pid << std::endl;
        kill(pid, SIGINT);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    std::cout << "Closing all terminals..." << std::endl;

    /*
    for (auto &pid : bash_ppids) {
        std::cout << "Sending SIGKILL to " << pid << std::endl;
        kill(pid, SIGKILL);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    */

    /*
     if (grconsole_pid != -1) {
         std::cout << "Closing grconsole monitor..." << std::endl;
         kill(grconsole_pid, SIGKILL);
     }
     */
}

void inputs_gather(std::filesystem::path parent_folder, std::string output_folder, std::filesystem::path output_path,
                   std::vector<std::filesystem::path> input_folders) {

    // TODO: Make it a function

    // std::string output_folder = output_path.filename();

    for (size_t n = 2; std::filesystem::exists(parent_folder / output_folder); n++) {
        output_folder = output_folder + "_" + std::to_string(n);
    }
    output_path = parent_folder / output_folder / "queue";

    std::filesystem::create_directories(output_path);

    for (auto &input_path : input_folders) {

        // Recursive iterator
        for (auto &p : std::filesystem::recursive_directory_iterator(input_path)) {

            if (p.is_directory() && p.path().filename().string() == "queue") {

                std::string copy_cmd = "rsync -a --exclude='.state' " + p.path().string() + "/ " + output_path.string();

                std::cout << copy_cmd << std::endl;

                run(copy_cmd);
            }
        }
    }
}
