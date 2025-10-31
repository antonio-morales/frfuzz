/* SPDX-License-Identifier: AGPL-3.0-only */
#include "afl.h"

#define MAX_NUM_TERMINALS 16

afl::afl() {
    name = "afl";

    // TODO: get from config
    executable_path = "/usr/local/bin/afl-fuzz";
}

afl::afl(std::string input_dir, std::string output_dir, std::string target, std::string args) {

    name = "afl";

    // TODO: get from config
    executable_path = "/usr/local/bin/afl-fuzz";

    input_directory = input_dir;
    output_directory = output_dir;

    target_path = target;
    cmdline = args;
}

afl::~afl() {}

uint32_t afl::get_crashes() {

    uint32_t value = 0;

    std::string tmp = get_stat("saved_crashes");

    if (tmp != "") {
        value = std::stoi(tmp);
    }

    return value;
}

void afl::parse_fuzzer_stats(std::string filecontent) {

    std::string line;
    std::istringstream iss(filecontent);

    while (std::getline(iss, line)) {
        std::string key;
        std::string value;

        std::istringstream iss2(line);
        std::getline(iss2, key, ':');
        std::getline(iss2, value, ':');

        key = trim(key);
        value = trim(value);

        stats[key] = value;
    }
}

void afl::load_stats() {

    if (output_directory == "") {
        return;
    }

    // Read output_directory + "fuzzer_stats"

    // Old scheme
    // TODO: Check AFL version to select the path
    // std::string filename = output_directory + "/fuzzer_stats";
    // std::string filecontent = read_file(filename);

    // New scheme
    std::string filename = output_directory + "default/fuzzer_stats";
    std::string filecontent = read_file(filename);

    // Parse filecontent into stats
    parse_fuzzer_stats(filecontent);
};

std::string afl::get_stat(std::string stat) {

    // Update stats
    load_stats();

    if (stats.count(stat) > 0) {
        return stats[stat];
    }

    // TODO: Remove temp solution
    /*
    if(stat == "saved_crashes"){
        return "0";
    }
    */

    return "";
}

enum AFL_MODE {
    MASTER,
    MASTER_UNIQUE,
    CMPLOG,
    COMPCOV,
    HAVOC,
    MOPT,
    EXPLOIT,
    DISABLE_TRIM,
    SEEK,
    RARE,
    MMOPT,
    LIN,
    COE,
    QUAD,
    OLD,
    CTX,    // AFL_LLVM_CTX=1
    CALLER, // AFL_LLVM_CALLER=1
    NGRAM,  // AFL_LLVM_NGRAM_SIZE
    NGRAM_UNIQUE,
    NGRAM_DET
};

struct AFL_OPTIONS {
    std::filesystem::path binary_path;
    std::string binary_args;
    std::filesystem::path input_folder;
    std::filesystem::path output_folder;
    bool deterministic;
    bool master;
    std::string name;
    std::string program_argv;
    size_t max_length;
    size_t timeout;
};

const std::vector<AFL_MODE> PROFILE_1 = {MASTER_UNIQUE, HAVOC};
const std::vector<AFL_MODE> PROFILE_10 = {HAVOC};

// Only LTO instrumentation
const std::vector<AFL_MODE> PROFILE_2 = {MASTER_UNIQUE, HAVOC, EXPLOIT, DISABLE_TRIM, SEEK, RARE, MMOPT, LIN, COE, QUAD, OLD};
const std::vector<AFL_MODE> PROFILE_20 = {HAVOC, EXPLOIT, DISABLE_TRIM, SEEK, RARE, MMOPT, LIN, COE, QUAD, OLD};

// LTO + LTO_COMPCOV + LLVM_CTX + LLVM_CALLER + LLVM_NGRAM
const std::vector<AFL_MODE> PROFILE_3 = {MASTER_UNIQUE, HAVOC, CTX,   CALLER, NGRAM, COMPCOV, EXPLOIT, DISABLE_TRIM,
                                         SEEK,          RARE,  MMOPT, LIN,    COE,   QUAD,    OLD};
const std::vector<AFL_MODE> PROFILE_30 = {HAVOC, CTX, CALLER, NGRAM, COMPCOV, EXPLOIT, DISABLE_TRIM, SEEK, RARE, MMOPT, LIN, COE, QUAD, OLD};

const std::vector<AFL_MODE> PROFILE_4 = {COMPCOV};

const std::vector<AFL_MODE> PROFILE_5 = {NGRAM_UNIQUE, NGRAM};
const std::vector<AFL_MODE> PROFILE_50 = {NGRAM};

const std::vector<AFL_MODE> PROFILE_6 = {MASTER}; // All are masters

struct AFL_CONFIG {
    std::filesystem::path binary_path;
    std::string arguments;
    std::vector<std::filesystem::path> crashes;
};

int AFL_get_version() {

    std::string version_str = "";

    std::string cmd = "afl-fuzz --version";
    std::string output = run(cmd);

    std::istringstream iss(output);

    std::string line;

    while (std::getline(iss, line)) {

        if (line.starts_with("afl-fuzz++")) {
            // Drop the first 10 characters
            version_str = line.substr(10);
            break;

        } else {
            std::cerr << "Error: Failed to get AFL version" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    int version;

    // Convert 4.09a to 409
    std::string major_str = version_str.substr(0, version_str.find_first_of('.'));
    int major = std::stoi(major_str) * 100;

    std::string minor_str = version_str.substr(version_str.find_first_of('.') + 1, 2);
    int minor = std::stoi(minor_str);

    version = major + minor;

    return version;
}

std::vector<int> launch_afl_instances(const std::filesystem::path src_folder, std::filesystem::path binary_rel_path, std::string binary_args,
                                      std::filesystem::path input_folder, std::filesystem::path output_folder,
                                      const std::vector<AFL_INSTANCE_CONFIG> instances, size_t max_length, size_t timeout, size_t memory_limit,
                                      std::string extension, std::vector<std::string> dictionary_paths, size_t cache_size) {

    std::vector<int> pids;

    size_t desktop = 0;
    size_t Pos_X = 0;
    size_t Pos_Y = 0;

    // Calculate the number of terminals that can fit on the screen
    size_t screen_width, screen_height;

    if (!get_screen_resolution(screen_width, screen_height)) {
        std::cerr << "Error: Failed to get screen resolution" << std::endl;
        exit(EXIT_FAILURE);
    }

    const int width_pixels = 916;
    const int height_pixels = 668;

    const int width_columns = 85;
    const int height_columns = 27;

    size_t num_terminals = divRoundClosest(screen_width, width_pixels);
    size_t num_rows = divRoundClosest(screen_height, height_pixels);

    size_t Xgap = screen_width / num_terminals;
    size_t Ygap = screen_height / num_rows;

    std::set<std::string> names;

    size_t current_instance = 1;

    int AFL_VERSION = AFL_get_version();
    std::cout << "AFL version: " << AFL_VERSION << std::endl;

    char buf[PATH_MAX];
    size_t num_bytes = readlink("/proc/self/exe", buf, PATH_MAX);
    // need to put te null byte at the end of the string
    buf[num_bytes] = '\0';
    std::string exec_path(buf);

    // Copy binary to a temp folder
    std::filesystem::path tmp_path = "/tmp";
    std::string random_name =
        "grconsole_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    std::filesystem::path random_path = tmp_path / random_name;
    std::filesystem::copy_file(exec_path, random_path);

    exec_path = random_path.string();
    exec_path += " monitor " + output_folder.string();
    int pid = launch_terminal(width_columns, height_columns + 10, Pos_X, Pos_Y, exec_path, "");

    Pos_X += Xgap;
    Pos_Y += 100;
    current_instance++;

    // Get the number of cores in the system
    int total_cores = std::thread::hardware_concurrency();
    std::cout << "Total cores: " << total_cores << std::endl;

    bool NO_AFFINITY = false;

    if (instances.size() >= total_cores) {
        NO_AFFINITY = true;
    }

    // Check how many MASTER instances are there
    size_t num_masters = 0;
    for (auto instance : instances) {
        if (instance.parallelism == AFL_PARALLELISM::MASTER) {
            num_masters++;
        }
    }

    size_t master_id = 0;

    for (auto instance : instances) {

        std::string name;

        std::string env = "AFL_TESTCACHE_SIZE=" + std::to_string(cache_size) + " AFL_IMPORT_FIRST=1";

        std::string cmd("afl-fuzz -i ");
        cmd += input_folder;

        cmd += " -o ";
        cmd += output_folder;

        if (instance.determinism == AFL_DETERMINISM::DETERMINISTIC) {
            if (AFL_VERSION < 420) {
                cmd += " -D";
            }
        } else {
            if (AFL_VERSION >= 420) {
                cmd += " -z";
            }
        }

        if (instance.parallelism == AFL_PARALLELISM::MASTER) {
            cmd += " -M";
            env += " AFL_FINAL_SYNC=1";
            name += "MASTER_";
            master_id++;
        } else if (instance.parallelism == AFL_PARALLELISM::SLAVE) {
            cmd += " -S";
        }

        std::filesystem::path instrumentation_folder;

        if (instance.instrumentation == AFL_INSTRUMENTATION::LTO) {
            instrumentation_folder = afl_builds[0] + "_lto";
            name += "LTO";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LTO_ASAN) {
            instrumentation_folder = afl_builds[1] + "_lto";
            name += "LTO_ASAN";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LTO_UBSAN) {
            instrumentation_folder = afl_builds[2] + "_lto";
            name += "LTO_UBSAN";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LTO_CFISAN) {
            instrumentation_folder = afl_builds[3] + "_lto";
            name += "LTO_CFISAN";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LTO_CMPLOG) {
            instrumentation_folder = afl_builds[4] + "_lto";
            name += "LTO_CMPLOG";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LTO_COMPCOV) {
            instrumentation_folder = afl_builds[5] + "_lto";
            name += "LTO_COMPCOV";

            // LLVM
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM) {
            instrumentation_folder = afl_builds[0] + "_llvm";
            name += "LLVM";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_ASAN) {
            instrumentation_folder = afl_builds[1] + "_llvm";
            name += "LLVM_ASAN";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_UBSAN) {
            instrumentation_folder = afl_builds[2] + "_llvm";
            name += "LLVM_UBSAN";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CFISAN) {
            instrumentation_folder = afl_builds[3] + "_llvm";
            name += "LLVM_CFISAN";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CMPLOG) {
            instrumentation_folder = afl_builds[4] + "_llvm";
            name += "LLVM_CMPLOG";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_COMPCOV) {
            instrumentation_folder = afl_builds[5] + "_llvm";
            name += "LLVM_COMPCOV";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CTX) {
            instrumentation_folder = afl_builds[6] + "_llvm";
            name += "LLVM_CTX";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CALLER) {
            instrumentation_folder = afl_builds[7] + "_llvm";
            name += "LLVM_CALLER";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_NGRAM) {
            instrumentation_folder = afl_builds[8] + "_llvm";
            name += "LLVM_NGRAM";
        } else if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM_NGRAM_ASAN) {
            instrumentation_folder = afl_builds[13] + "_llvm";
            name += "LLVM_NGRAM_ASAN";
        }

        std::string strategy;
        if (instance.strategy == AFL_STRATEGY::EXPLORE) {
            strategy += " -P explore";
            name += "_EXPLORE";
        } else if (instance.strategy == AFL_STRATEGY::EXPLOIT) {
            strategy += " -P exploit";
            name += "_EXPLOIT";
        }

        std::string schedule;
        if (instance.power_schedule == AFL_POWER_SCHEDULE::EXPLORE) {
            schedule += " -p explore";
            name += "_explore";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::EXPLOIT) {
            schedule += " -p exploit";
            name += "_exploit";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::SEEK) {
            schedule += " -p seek";
            name += "_seek";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::RARE) {
            schedule += " -p rare";
            name += "_rare";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::MMOPT) {
            schedule += " -p mmopt";
            name += "_mmopt";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::LIN) {
            schedule += " -p lin";
            name += "_lin";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::COE) {
            schedule += " -p coe";
            name += "_coe";
        } else if (instance.power_schedule == AFL_POWER_SCHEDULE::QUAD) {
            schedule += " -p quad";
            name += "_quad";
        }

        // TODO: Check if NO_TRIM Is working
        if (instance.trim == AFL_TRIM::DISABLE) {
            env += " AFL_DISABLE_TRIM=1";
            name += "_noTrim";
        }

        std::string queue;
        if (instance.queue_selection == AFL_QUEUE_SELECTION::SEQUENTIAL) {
            queue += " -Z";
            name += "_sequential";
        }

        size_t n = 2;
        std::string name_tmp = name;
        while (names.contains(name_tmp)) {
            name_tmp = name + std::to_string(n);
            n++;
        }
        name = name_tmp;
        names.insert(name);

        if (num_masters > 1) {
            name += ":" + std::to_string(master_id) + "/" + std::to_string(num_masters);
        }

        cmd += " " + name + strategy + schedule + queue;

        if (max_length > 0) {
            cmd += " -G ";
            cmd += std::to_string(max_length);
        }

        if (instance.instrumentation == AFL_INSTRUMENTATION::LTO_ASAN || instance.instrumentation == AFL_INSTRUMENTATION::LTO_UBSAN ||
            instance.instrumentation == AFL_INSTRUMENTATION::LTO_CFISAN || instance.instrumentation == AFL_INSTRUMENTATION::LLVM_ASAN ||
            instance.instrumentation == AFL_INSTRUMENTATION::LLVM_UBSAN || instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CFISAN) {
            // No memory limit for ASAN, UBSAN and CFISAN
        } else {
            cmd += " -m ";
            if (memory_limit == 0) {
                cmd += "none";
            } else {
                cmd += std::to_string(memory_limit);
            }
        }

        std::filesystem::path folder = instrumentation_folder.string();

        if (!std::filesystem::exists(folder)) {
            std::cerr << "Instrumentation folder " << folder << " does not exist" << std::endl;
            std::cerr << "You need to compile it first" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (dictionary_paths.size() > 0) { // If dictionary is provided

            for (auto dictionary_path : dictionary_paths) {

                // Check if dictionary exists
                if (!std::filesystem::exists(dictionary_path)) {
                    std::cerr << "Dictionary file does not exist: " << dictionary_path << std::endl;
                    exit(EXIT_FAILURE);
                }

                cmd += " -x ";
                cmd += dictionary_path;
            }

            // We disable AUTODICT
            env += " AFL_NO_AUTODICT=1";

        } else { // If no dictionary is provided, use AUTODICT

            if (instance.instrumentation == AFL_INSTRUMENTATION::LLVM || instance.instrumentation == AFL_INSTRUMENTATION::LLVM_ASAN ||
                instance.instrumentation == AFL_INSTRUMENTATION::LLVM_UBSAN || instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CFISAN ||
                instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CMPLOG || instance.instrumentation == AFL_INSTRUMENTATION::LLVM_COMPCOV ||
                instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CTX || instance.instrumentation == AFL_INSTRUMENTATION::LLVM_CALLER ||
                instance.instrumentation == AFL_INSTRUMENTATION::LLVM_NGRAM) {

                // Check if AUTODICT exists
                if (std::filesystem::exists(folder / "AUTODICT.txt")) {
                    cmd += " -x ";
                    cmd += folder.string() + "/AUTODICT.txt";
                }
            }
        }

        cmd += " -t ";
        cmd += std::to_string(timeout);

        if (extension != "") {
            cmd += " -e ";
            cmd += extension;
        }

        cmd += " -- ";

        cmd += std::filesystem::absolute(folder / binary_rel_path);
        cmd += " ";
        cmd += binary_args;

        std::cout << "cmd: " << cmd << std::endl << std::endl;

        // std::string wmctrl_cmd("wmctrl -r :ACTIVE: -t ");
        // wmctrl_cmd += std::to_string(desktop);
        // int pid = launch_terminal(width_columns, height_columns, Pos_X, Pos_Y, wmctrl_cmd + "; " + cmd.c_str(), env);

        if (current_instance < MAX_NUM_TERMINALS) {

            int pid = launch_terminal(width_columns, height_columns, Pos_X, Pos_Y, cmd.c_str(), env);
            // int pid;

            pids.push_back(pid);

            Pos_X += Xgap;

            if (current_instance % num_terminals == 0) {
                Pos_X = 0;
                Pos_Y += Ygap;
            }

            if (current_instance % (num_terminals * num_rows) == 0) {
                Pos_Y = 0;
                Pos_X = 0;

                desktop += 1;

                sleep(1);
                switch_to_desktop(desktop);
            }

        } else {

            env += " AFL_NO_UI=1";

            if (NO_AFFINITY) {
                env += " AFL_NO_AFFINITY=1";
            }

            int pid = launch_shell(cmd, env);

            pids.push_back(pid);
        }

        // global_Seed++;

        current_instance++;

        std::cout << "Waiting for the next one" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return pids;
}

std::vector<int> launch_afl_instances_OLD(const std::filesystem::path src_folder, std::filesystem::path binary_rel_path, std::string binary_args,
                                          std::filesystem::path input_folder, std::filesystem::path output_folder,
                                          const std::vector<AFL_MODE> profile, size_t max_length, size_t timeout, size_t memory_limit, size_t cores) {

    std::vector<int> pids;

    size_t desktop = 0;
    size_t Pos_X = 0;
    const size_t Pos_Y = 200;

    // std::vector<AFL_MODE> profile = {MASTER, CMPLOG, COMPCOV, HAVOC, MOPT, EXPLOIT, DISABLE_TRIM, SEEK, RARE, MMOPT, LIN, COE, QUAD, OLD};

    int master = 0;
    int cmplog = 1;
    int compcov = 1;
    int havoc = 1;
    int exploit = 1;
    int disableTrim = 1;
    int seek = 1;
    int rare = 1;
    int mmopt = 1;
    int lin = 1;
    int coe = 1;
    int quad = 1;
    int old = 1;
    int ctx = 1;
    int caller = 1;
    int ngram = 1;

    // Get the number of cores in the system
    int total_cores = std::thread::hardware_concurrency();
    std::cout << "Total cores: " << total_cores << std::endl;

    std::vector<AFL_MODE> instances;

    // int circular_index = 0;

    // for (int current_core = 0;

    for (int circular_index = 0; instances.size() < cores; circular_index++) {

        AFL_MODE mode = profile[circular_index % profile.size()];

        if (mode == MASTER_UNIQUE) {

            if (instances.size() == 0) {
                mode = MASTER;
            } else {
                continue;
            }
        } else if (mode == NGRAM_UNIQUE) {

            if (instances.size() == 0) {
                mode = NGRAM_DET;
            } else {
                continue;
            }
        }

        if (mode == MASTER) {
            master++;
        }

        instances.push_back(mode);
    }

    int current_core = 0;

    for (auto mode : instances) {

        // AFL_MODE mode = profile[current_core % profile.size()];

        /*
        if (current_core >= cores) {
            break;
        }
        */

        std::string env = "AFL_TESTCACHE_SIZE=600 AFL_IMPORT_FIRST=1";

        std::string name;
        std::string suffix;

        bool deterministic = false;
        bool isMaster = false;

        std::string strategy = "";

        bool isCmplog = false;
        std::string cmplog_opts = "";

        bool OldQueue = false;

        std::string power_schedule = "";

        std::string MOpt = "";

        if (current_core == 0) {
            // deterministic = true;
            isMaster = true;
        }

        switch (mode) {

        case MASTER:

            /*
            if(current_core > 0) {
                cores++;
                continue;
            }
            */

            static size_t master_index = 1;

            name = "MASTER";
            if (master > 1) {
                name += std::to_string(master_index) + ":" + std::to_string(master_index) + "/" + std::to_string(master);
            }
            suffix = afl_builds[0];
            deterministic = true;
            isMaster = true;
            master_index++;
            break;

        case CMPLOG:
            name = "CMPLOG";
            if (cmplog > 1) {
                name += std::to_string(cmplog);
            }
            suffix = afl_builds[4];
            isCmplog = true;
            cmplog++;
            break;

        case COMPCOV:
            name = "COMPCOV";
            if (compcov > 1) {
                name += std::to_string(compcov);
            }
            suffix = afl_builds[5];
            compcov++;
            break;

        case HAVOC:
            name = "Havoc";
            if (havoc > 1) {
                name += std::to_string(havoc);
            }
            suffix = afl_builds[0];
            havoc++;
            break;

        case MOPT:
            /*
            name = "MOpt";
            suffix = afl_builds[0];
            MOpt = "0";
            break;
            */

        case EXPLOIT:
            name = "Exploit";
            if (exploit > 1) {
                name += std::to_string(exploit);
            }
            suffix = afl_builds[0];
            strategy = "exploit";
            exploit++;
            break;

        case DISABLE_TRIM:
            name = "DisableTrim";
            if (disableTrim > 1) {
                name += std::to_string(disableTrim);
            }
            suffix = afl_builds[0];
            env += " AFL_DISABLE_TRIM=1";
            disableTrim++;
            break;

        case SEEK:
            name = "Seek";
            if (seek > 1) {
                name += std::to_string(seek);
            }
            suffix = afl_builds[0];
            power_schedule = "seek";
            seek++;
            break;

        case RARE:
            name = "Rare";
            if (rare > 1) {
                name += std::to_string(rare);
            }
            suffix = afl_builds[0];
            power_schedule = "rare";
            rare++;
            break;

        case MMOPT:
            name = "MMopt";
            if (mmopt > 1) {
                name += std::to_string(mmopt);
            }
            suffix = afl_builds[0];
            power_schedule = "mmopt";
            mmopt++;
            break;

        case LIN:
            name = "Lin";
            if (lin > 1) {
                name += std::to_string(lin);
            }
            suffix = afl_builds[0];
            power_schedule = "lin";
            lin++;
            break;

        case COE:
            name = "Coe";
            if (coe > 1) {
                name += std::to_string(coe);
            }
            suffix = afl_builds[0];
            power_schedule = "coe";
            coe++;
            break;

        case QUAD:
            name = "Quad";
            if (quad > 1) {
                name += std::to_string(quad);
            }
            suffix = afl_builds[0];
            power_schedule = "quad";
            quad++;
            break;

        case OLD:
            name = "OldQueue";
            if (old > 1) {
                name += std::to_string(old);
            }
            suffix = afl_builds[0];
            OldQueue = true;
            old++;
            break;

        case CTX:
            name = "CTX";
            if (ctx > 1) {
                name += std::to_string(ctx);
            }
            suffix = afl_builds[6];
            env += " AFL_LLVM_CTX=1";
            ctx++;
            break;

        case CALLER:
            name = "CALLER";
            if (caller > 1) {
                name += std::to_string(caller);
            }
            suffix = afl_builds[7];
            env += " AFL_LLVM_CALLER=1";
            caller++;
            break;

        case NGRAM:
            name = "NGRAM";
            if (ngram > 1) {
                name += std::to_string(ngram);
            }
            suffix = afl_builds[8];
            env += " AFL_LLVM_NGRAM_SIZE=6";
            ngram++;
            break;

        case NGRAM_DET:
            deterministic = true;
            name = "NGRAM";
            if (ngram > 1) {
                name += std::to_string(ngram);
            }
            suffix = afl_builds[8];
            env += " AFL_LLVM_NGRAM_SIZE=6";
            ngram++;
            break;
        }

        std::filesystem::path folder = src_folder.string() + suffix;

        std::string cmd("afl-fuzz -i ");
        cmd += input_folder;

        cmd += " -o ";
        cmd += output_folder;

        if (deterministic) {
            cmd += " -D";
        }

        if (isMaster) {
            cmd += " -M";
            env += " AFL_FINAL_SYNC=1";
        } else {
            cmd += " -S";
        }

        cmd += " " + name;

        if (isCmplog) {
            cmd += " -c " + (folder / binary_rel_path).string() + " -m none";
            if (cmplog_opts != "") {
                cmd += " -l " + cmplog_opts;
            }

            binary_rel_path = src_folder.string() + afl_builds[0] + "/binutils/readelf"; // TODO: Fix this
        }

        if (strategy != "") {
            cmd += " -P " + strategy;
        }

        if (MOpt != "") {
            cmd += " -L " + MOpt;
        }

        if (power_schedule != "") {
            cmd += " -p " + power_schedule;
        }

        if (OldQueue) {
            cmd += " -Z";
        }

        /*
        if (!global_Master) {

            cmd += " -M master";

            // pid = launch_terminal(global_X, global_Y, "afl-fuzz -i $HOME/fuzzing_xpdf/pdf_examples/ -o $HOME/fuzzing_xpdf/out/ -M master -s 123 --
        $HOME/fuzzing_xpdf/install/bin/pdftotext @@ $HOME/fuzzing_xpdf/output"); global_Master = 1; } else {

            cmd += " -S slave";
            cmd += std::to_string(global_Slave);
            global_Slave++;

            // pid = launch_terminal(global_X, global_Y, "afl-fuzz -i $HOME/fuzzing_xpdf/pdf_examples/ -o $HOME/fuzzing_xpdf/out/ -S slave1 -s 123 --
        $HOME/fuzzing_xpdf/install/bin/pdftotext @@ $HOME/fuzzing_xpdf/output");
        }
        */

        // cmd += " -s ";
        //  cmd += std::to_string(global_Seed);

        cmd += " -G ";
        cmd += std::to_string(max_length);

        cmd += " -m ";
        cmd += std::to_string(memory_limit);

        cmd += " -t ";
        cmd += std::to_string(timeout);

        cmd += " -- ";
        cmd += std::filesystem::absolute(folder / binary_rel_path);
        cmd += " ";
        cmd += binary_args;

        std::cout << "cmd: " << cmd << std::endl;
        // std::cin.get();

        const int width = 85;
        const int height = 27;

        // std::string wmctrl_cmd("wmctrl -s ");
        // wmctrl_cmd += std::to_string(desktop);
        // int pid = launch_terminal(width, height, Pos_X, Pos_Y, wmctrl_cmd + "; " + cmd.c_str(), env);

        int pid = launch_terminal(width, height, Pos_X, Pos_Y, cmd.c_str(), env);
        // int pid;
        pids.push_back(pid);

        Pos_X += 100;

        if (Pos_X > 2000) {
            Pos_X = 0;
            desktop += 1;
        }
        // global_Seed++;

        std::cout << "Waiting for the next one" << std::endl;
        sleep(5);
        // usleep(500000);

        current_core++;
    }

    return pids;
}

bool AFL_checkFolder(std::filesystem::path folder) {

    // TODO: Detection algorithm: Check if > 50% of the folders have the required files

    // Count the number of folders inside the folder
    int numFolders = 0;
    for (auto &p : std::filesystem::directory_iterator(folder)) {
        if (p.is_directory()) {
            numFolders++;
        }
    }

    // std::vector<std::filesystem::path> cmdline_files;

    for (auto &p : std::filesystem::directory_iterator(folder)) {

        if (p.is_directory()) {

            bool cmdline_found = false;
            bool fuzz_bitmap_found = false;
            bool fuzzer_setup_found = false;
            bool fuzzer_stats_found = false;
            bool plot_data_found = false;

            // Check files inside the directory
            for (auto &q : std::filesystem::directory_iterator(p)) {

                if (q.is_regular_file()) {

                    // Look for the cmdline file
                    if (q.path().filename().string() == "cmdline") {
                        cmdline_found = true;

                    } else if (q.path().filename().string() == "fuzz_bitmap") {
                        fuzz_bitmap_found = true;

                    } else if (q.path().filename().string() == "fuzzer_setup") {
                        fuzzer_setup_found = true;

                    } else if (q.path().filename().string() == "fuzzer_stats") {
                        fuzzer_stats_found = true;

                    } else if (q.path().filename().string() == "plot_data") {
                        plot_data_found = true;
                    }
                }
            }

            if (cmdline_found + fuzz_bitmap_found + fuzzer_setup_found + fuzzer_stats_found + plot_data_found >= 3) {
                return true;
            }
        }
    }

    return false;
}

bool AFL_read_config(const std::filesystem::path AFL_folder, AFL_CONFIG &config) {

    if (AFL_checkFolder(AFL_folder) == false) {
        std::cerr << "The folder " << AFL_folder << " does not look like a valid AFL output folder" << std::endl;
        return false;
    }

    bool config_found = false;

    for (auto &p : std::filesystem::directory_iterator(AFL_folder)) {

        if (p.is_directory()) {

            for (auto &q : std::filesystem::directory_iterator(p)) {

                if (!config_found & q.is_regular_file() && q.path().filename().string() == "cmdline") {

                    std::ifstream file(q.path());

                    // Read the first line
                    std::string line;

                    std::getline(file, line);

                    config.binary_path = line;

                    // Read the rest of the file
                    while (std::getline(file, line)) {
                        config.arguments += " " + line;
                    }

                    // Delete the @@ from the arguments
                    config.arguments.erase(std::remove(config.arguments.begin(), config.arguments.end(), '@'), config.arguments.end());

                    // TODO: Implement the @@ automatic replacement

                    config_found = true;

                } else if (q.is_directory() && q.path().filename().string() == "crashes") {

                    for (auto &r : std::filesystem::directory_iterator(q)) {

                        if (r.is_regular_file() && r.path().filename().string() != "README.txt") {

                            config.crashes.push_back(r.path());
                        }
                    }
                }
            }
        }
    }

    return config_found;
}

std::vector<std::filesystem::path> AFL_get_crashes(const std::filesystem::path AFL_folder) {

    std::vector<std::filesystem::path> crashes;

    for (auto &p : std::filesystem::recursive_directory_iterator(AFL_folder)) {

        if (p.is_directory() && p.path().filename().string() == "crashes") {

            for (auto &q : std::filesystem::directory_iterator(p)) {

                if (q.is_regular_file() && q.path().filename().string() != "README.txt") {

                    crashes.push_back(q.path());
                }
            }
        }
    }

    return crashes;
}

void fuzz_afl(std::string profileFile, size_t cores, std::string input_path, std::filesystem::path output_path, size_t max_length, size_t timeout,
              size_t memory_limit, std::string extension, std::vector<std::string> dictionary_paths, size_t cache_size, const FRglobal &ctx) {

    std::vector<AFL_INSTANCE_CONFIG> instances;

    if (!PROFILE_FILES.contains(profileFile)) {

        std::cerr << "Error: Profile file does not exist" << std::endl;
        std::cout << std::endl;
        std::cout << "Available profiles are:" << std::endl;
        std::cout << std::endl;

        std::vector<std::string> profiles;

        for (auto &it : PROFILE_FILES) {
            profiles.push_back(it.first);
        }

        std::sort(profiles.begin(), profiles.end());

        for (auto &profile : profiles) {
            std::cout << "- " << profile << std::endl;
        }

        exit(1);
    }

    std::istringstream file(PROFILE_FILES.at(profileFile));

    // Read profile file line by line
    std::string line;
    for (int n = 0; n < cores && std::getline(file, line); n++) {

        AFL_INSTANCE_CONFIG instance;

        std::string token;

        size_t start_pos = 0;
        size_t end_pos = 0;

        for (int i = 0; i < 7; i++) {

            if (start_pos >= line.size()) {
                std::cerr << "Error: Profile file is not well formatted" << std::endl;
                exit(1);
            }

            end_pos = line.find(';', start_pos);
            if (end_pos == std::string::npos) {
                std::cerr << "Error: Profile file is not well formatted" << std::endl;
                exit(1);
            }

            token = line.substr(start_pos, end_pos - start_pos);

            switch (i) {
            case 0: {
                if (token == "NONE") {
                    instance.parallelism = AFL_PARALLELISM::NONE;
                } else if (token == "MASTER") {
                    instance.parallelism = AFL_PARALLELISM::MASTER;
                } else if (token == "SLAVE") {
                    instance.parallelism = AFL_PARALLELISM::SLAVE;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }

            case 1: {
                if (token == "HAVOC") {
                    instance.determinism = AFL_DETERMINISM::HAVOC;
                } else if (token == "DETERMINISTIC") {
                    instance.determinism = AFL_DETERMINISM::DETERMINISTIC;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }

            case 2: {
                if (token == "LTO") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LTO;
                } else if (token == "LTO_ASAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LTO_ASAN;
                } else if (token == "LTO_UBSAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LTO_UBSAN;
                } else if (token == "LTO_CFISAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LTO_CFISAN;
                } else if (token == "LTO_CMPLOG") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LTO_CMPLOG;
                } else if (token == "LTO_COMPCOV") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LTO_COMPCOV;

                } else if (token == "LLVM") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM;
                } else if (token == "LLVM_ASAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_ASAN;
                } else if (token == "LLVM_UBSAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_UBSAN;
                } else if (token == "LLVM_CFISAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_CFISAN;
                } else if (token == "LLVM_CMPLOG") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_CMPLOG;
                } else if (token == "LLVM_COMPCOV") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_COMPCOV;
                } else if (token == "LLVM_CTX") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_CTX;
                } else if (token == "LLVM_CALLER") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_CALLER;
                } else if (token == "LLVM_NGRAM") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_NGRAM;
                } else if (token == "LLVM_NGRAM_ASAN") {
                    instance.instrumentation = AFL_INSTRUMENTATION::LLVM_NGRAM_ASAN;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }

            case 3: {
                if (token == "NONE") {
                    instance.strategy = AFL_STRATEGY::NONE;
                } else if (token == "EXPLORE") {
                    instance.strategy = AFL_STRATEGY::EXPLORE;
                } else if (token == "EXPLOIT") {
                    instance.strategy = AFL_STRATEGY::EXPLOIT;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }

            case 4: {
                if (token == "DEFAULT") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::DEFAULT;
                } else if (token == "EXPLORE") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::EXPLORE;
                } else if (token == "EXPLOIT") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::EXPLOIT;
                } else if (token == "COE") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::COE;
                } else if (token == "LIN") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::LIN;
                } else if (token == "QUAD") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::QUAD;
                } else if (token == "MMOPT") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::MMOPT;
                } else if (token == "SEEK") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::SEEK;
                } else if (token == "RARE") {
                    instance.power_schedule = AFL_POWER_SCHEDULE::RARE;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }

            case 5: {
                if (token == "DEFAULT") {
                    instance.trim = AFL_TRIM::DEFAULT;
                } else if (token == "DISABLE") {
                    instance.trim = AFL_TRIM::DISABLE;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }

            case 6: {
                if (token == "DEFAULT") {
                    instance.queue_selection = AFL_QUEUE_SELECTION::DEFAULT;
                } else if (token == "SEQUENTIAL") {
                    instance.queue_selection = AFL_QUEUE_SELECTION::SEQUENTIAL;
                } else {
                    std::cerr << "Error: Profile file is not well formatted" << std::endl;
                    exit(1);
                }
                break;
            }
            }

            start_pos = end_pos + 1;
        }

        instances.push_back(instance);
    }

    launch_afl_instances(ctx.campaign->src_folder, ctx.campaign->binary_rel_path, ctx.campaign->binary_args, input_path, output_path, instances,
                         max_length, timeout, memory_limit, extension, dictionary_paths, cache_size);
}
