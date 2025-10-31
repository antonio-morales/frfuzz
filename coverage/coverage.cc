/* SPDX-License-Identifier: AGPL-3.0-only */
#include "coverage.h"

/*
class LCOVfunction
{

private:
    std::string funcName;

    std::vector<std::string> parameters;

    std::string path;

    std::vector<LCOVline> lines;

    text_pos body_start;

    int uncoveredLines;


}
*/

int genhtml_get_version() {

    std::string version_str = "";

    std::string cmd = "genhtml --version";
    std::string output = run(cmd);

    std::istringstream iss(output);

    std::string line;

    while (std::getline(iss, line)) {

        if (line.starts_with("genhtml: LCOV version ")) {
            // Drop the first 22 characters
            version_str = line.substr(22);
            break;

        } else {
            std::cerr << "Error: Failed to get genhtml version" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    int version;

    // Convert 1.16 to 116
    std::string major_str = version_str.substr(0, version_str.find_first_of('.'));
    int major = std::stoi(major_str) * 100;

    std::string minor_str = version_str.substr(version_str.find_first_of('.') + 1, 3);
    // Remove the '-' character from minor_str
    minor_str.erase(std::remove(minor_str.begin(), minor_str.end(), '-'), minor_str.end());
    int minor = std::stoi(minor_str);

    version = major + minor;

    return version;
}

void coverage(std::vector<std::filesystem::path> output_folders, const FRglobal &ctx, bool html_report) {

    // The init_folder is the coverage folder
    std::filesystem::path init_folder = ctx.campaign->campaign_path / "__COV";

    // TODO: Get the "binary path" and "arguments" from the cmdline file
    std::filesystem::path binary_path;
    std::string arguments;

    binary_path = init_folder / ctx.campaign->binary_rel_path;
    binary_path = std::filesystem::canonical(binary_path); // Check if the file exists

    debug() << "Binary path: " << binary_path << std::endl;

    // TODO: Check if the binary is executable
    if (!std::filesystem::is_regular_file(binary_path)) {
        std::cerr << "Binary " << binary_path << " does not exist or is not executable" << std::endl;
        exit(EXIT_FAILURE);
    }

    arguments = " " + ctx.campaign->binary_args;

    std::filesystem::path baseline_cov;
    std::filesystem::path current_cov;
    std::filesystem::path html_folder;

    if (output_folders.size() == 1) {
        baseline_cov = output_folders[0] / "app.info";
        current_cov = output_folders[0] / "app2.info";
        html_folder = output_folders[0] / "html-coverage";
    } else {

        // Use the parent folder
        std::filesystem::path parent_folder = output_folders[0].parent_path();
        baseline_cov = parent_folder / "app.info";
        current_cov = parent_folder / "app2.info";
        html_folder = parent_folder / "html-coverage";
    }
    // std::filesystem::path baseline_cov = output_folder / "app.info";
    // std::filesystem::path current_cov = output_folder / "app2.info";
    // std::filesystem::path html_folder = output_folder / "html-coverage";

    // Check if the folder exists
    // TODO: Refactor to a function
    if (std::filesystem::exists(html_folder)) {
        html_folder +=
            "_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    }

    // Check genhtml version
    int genhtml_version = genhtml_get_version();
    debug() << "genhtml version: " << genhtml_version << std::endl << std::endl;

    // Delete previous coverage files
    std::filesystem::remove(baseline_cov);
    std::filesystem::remove(current_cov);
    // std::filesystem::remove_all(html_folder);

    // Create a tmp folder in /tmp to store the coverage files. The folder name has to be different each time
    // std::filesystem::path tmp_folder = "/tmp/coverage_" +
    // std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    // std::filesystem::create_directory(tmp_folder);

    std::string command = "lcov --zerocounters --directory " + init_folder.string();
    std::string output = run(command);

    command = "lcov --capture --initial --ignore-errors source --directory " + init_folder.string() + " --output-file " + baseline_cov.string();
    output = run(command);

    debug() << output << std::endl;

    if (output.find("Finished .info-file creation") == std::string::npos) {
        std::cerr << "Error: lcov initial capture failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    debug() << "Gathering all input files..." << std::endl;

    int num_files = 0;
    std::vector<std::filesystem::path> input_files;

    for (auto &output_folder : output_folders) {

        // Gather coverage from all files inside "queue" folders
        for (auto &p : std::filesystem::recursive_directory_iterator(output_folder)) {

            // Check if it is a queue folder
            if (p.path().filename().string().starts_with("queue") && p.is_directory()) {

                // Iterate over all files inside queue folder
                for (auto &r : std::filesystem::directory_iterator(p)) {

                    // Check if it is a file
                    if (r.is_regular_file()) {

                        // std::filesystem::path new_filename = p.path().filename().string() + "_" + r.path().filename().string();

                        // Copy r to tmp_folder
                        // std::filesystem::copy(r, tmp_folder / new_filename);

                        // Add the file to the input_files vector
                        input_files.push_back(r.path());

                        // if (num_files % 5000 == 0)
                        // std::cout << "Executed " << num_files << " files" << std::endl;

                        num_files++;
                    }
                }
            }
        }
    }

    debug() << "Total input files: " << num_files << std::endl << std::endl;

    debug() << "Executing all input files..." << std::endl;

    auto begin = std::chrono::high_resolution_clock::now();

    const size_t timeout = 1000;

    command = binary_path.string() + arguments;

    debug() << "Command: " << command << std::endl;

    std::vector<std::thread> threads;

    int numExecsPerThread = num_files / ctx.numThreads;
    int remainingExecs = num_files % ctx.numThreads;

    for (size_t i = 0; i < ctx.numThreads; ++i) {

        size_t posInicial = i * numExecsPerThread;

        size_t posFinal = posInicial + numExecsPerThread - 1;
        if (i == (ctx.numThreads - 1))
            posFinal += remainingExecs;

        threads.push_back(std::thread(run_thread, i, input_files, posInicial, posFinal, command, timeout));
    }

    for (auto &th : threads) {
        th.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    debug() << "Total time: " << std::dec << seconds << "secs" << std::endl;

    command = "lcov --no-checksum --directory " + init_folder.string() + " --capture --output-file " + current_cov.string();
    output = run(command);
    debug() << command << std::endl;
    debug() << output << std::endl << std::endl;

    if (output.find("Finished .info-file creation") == std::string::npos) {
        std::cerr << "Error: lcov initial capture failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (html_report) {

        command = "genhtml --highlight";

        if (genhtml_version >= 200) {
            command += " --ignore-errors unmapped";
        }

        command += " --legend -output-directory " + html_folder.string() + " " + current_cov.string();

        output = run(command);
        debug() << command << std::endl;
        debug() << output << std::endl << std::endl;
    }
}

std::mutex mutex, mutex2, mutex3, mutex4;

void break_thread(size_t thread_id, const std::vector<std::filesystem::path> &inputs, size_t posInicial, size_t posFinal, std::string command,
                  std::string args, std::vector<std::filesystem::path> &paths) {

    size_t num_elements = posFinal - posInicial + 1;

    for (int i = posInicial; i <= posFinal; i++) {

        if (thread_id == 0 && i % 10 == 0) {
            std::cout << "Current run: " << i - posInicial + 1 << " / " << num_elements << std::endl;
        }

        std::string cmd = command;

        std::string arguments = args;

        arguments = std::regex_replace(arguments, std::regex("@@"), bash_escape(inputs[i].string()));

        cmd += " " + arguments;

        // std::cout << "cmd: " << cmd << std::endl;

        std::string output = run(cmd);

        // Find in output
        if (output.find("hit Breakpoint 1") != std::string::npos) {
            mutex.lock();
            paths.push_back(inputs[i]);
            std::cout << inputs[i] << std::endl;
            std::cout << cmd << std::endl;
            std::cout << output << std::endl;
            std::cout << std::endl;
            mutex.unlock();
        }
    }
}

void do_break(std::string breakpoint, std::vector<std::filesystem::path> output_folders, const FRglobal &ctx) {

    // The init_folder is the coverage folder
    std::filesystem::path init_folder = "__COV";

    std::filesystem::path binary_path = init_folder / ctx.campaign->binary_rel_path;

    int num_files = 0;
    std::vector<std::filesystem::path> input_files;

    for (auto &output_folder : output_folders) {

        // Gather coverage from all files inside "queue" folders
        for (auto &p : std::filesystem::recursive_directory_iterator(output_folder)) {

            // Check if it is a queue folder
            if (p.path().filename().string() == "queue" && p.is_directory()) {

                // Iterate over all files inside queue folder
                for (auto &r : std::filesystem::directory_iterator(p)) {

                    // Check if it is a file
                    if (r.is_regular_file()) {

                        // Add the file to the input_files vector
                        input_files.push_back(r.path());

                        num_files++;
                    }
                }
            }
        }
    }

    std::cout << "Total input files: " << num_files << std::endl;
    std::cout << std::endl;

    std::cout << "Executing all input files..." << std::endl;

    std::string cmd = "gdb";

    cmd += " --batch"; // Batch mode

    cmd += " -ex 'set breakpoint pending on'";

    cmd += " -ex 'break " + breakpoint + "'"; // Set the breakpoint

    cmd += " -ex 'run'";

    cmd += " -ex 'continue'"; // Continue the execution

    cmd += " -ex 'quit'"; // Continue the execution

    cmd += " --args " + binary_path.string(); // Load the binary"

    std::vector<std::filesystem::path> paths;

    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    size_t numExecsPerThread = input_files.size() / ctx.numThreads;
    int remainingExecs = input_files.size() % ctx.numThreads;

    if (numExecsPerThread == 0) {

        threads.push_back(std::thread(break_thread, 0, input_files, 0, remainingExecs - 1, cmd, ctx.campaign->binary_args, std::ref(paths)));

    } else {

        for (size_t i = 0; i < ctx.numThreads; ++i) {

            size_t posInicial = i * numExecsPerThread;

            size_t posFinal = posFinal = posInicial + numExecsPerThread - 1;

            if (i == (ctx.numThreads - 1)) // La \FAltima hebra asume el restante de palabras que no es divisible
                posFinal += remainingExecs;

            threads.push_back(std::thread(break_thread, i, input_files, posInicial, posFinal, cmd, ctx.campaign->binary_args, std::ref(paths)));
        }
    }

    for (auto &th : threads) {
        th.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();

    // Display the summary

    std::cout << std::endl;

    std::cout << "Total time: " << std::dec << seconds << "secs" << std::endl;
}
