/* SPDX-License-Identifier: AGPL-3.0-only */
#include "grconsole.h"

void auto_update() {

    const std::string remote_host = "10.0.0.1";
    const std::string file_names[] = {"grconsole", "ULIEngine"};
    const std::string updater_name = "updater";

    const std::filesystem::path home_dir = get_home_dir();

    const std::filesystem::path local_path = home_dir / "bin";

    for (auto file_name : file_names) {

        const std::filesystem::path local_file = local_path / file_name;
        const std::filesystem::path updater_path = local_path / updater_name;

        std::filesystem::path tmp_dir = home_dir / "tmp";

        if (!std::filesystem::exists(tmp_dir)) {
            std::filesystem::create_directory(tmp_dir);
        }

        const std::filesystem::path tmp_file = tmp_dir / file_name;

        if (std::filesystem::exists(tmp_file)) {
            std::filesystem::remove(tmp_file);
        }

        std::cout << "Checking for new versions of " << file_name << " in " << remote_host << "..." << std::endl;

        // TODO: Use a libcurl function instead of wget
        std::string command = "cd " + tmp_dir.string() + "; wget http://" + remote_host + "/" + file_name;
        std::string result = run(command.c_str());
        // std::cout << result << std::endl;

        // Wait for the download to finish
        // std::this_thread::sleep_for(std::chrono::seconds(3));

        std::cout << std::endl;

        // search result for "‘grconsole’ saved"
        if (result.find("awaiting response... 200 OK") == std::string::npos) {
            std::cerr << "Error: Connection failed" << std::endl;
            exit(EXIT_FAILURE);
        } else {
            std::cout << "Successfully connected to " << remote_host << std::endl;
        }

        std::cout << std::endl;

        const auto local_date = last_modified(local_file);
        std::cout << "Current version: " << std::asctime(std::localtime(&local_date));

        const auto remote_date = last_modified(tmp_file);
        std::cout << "Remote version: " << std::asctime(std::localtime(&remote_date)) << std::endl;

        if (local_date < remote_date) {

            execl(updater_path.c_str(), "updater", tmp_file.c_str(), local_file.c_str(), NULL);

        } else {
            std::cout << "Local file is up to date." << std::endl << std::endl;
        }
    }
}

void print_help(char *argv[], std::string command = "") {

    if (command == "") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <list>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <install> <campaign> ..." << std::endl;
        std::cout << "Usage: " << argv[0] << " <tree> <queue_folder> " << std::endl;
        std::cout << "Usage: " << argv[0] << " <setup> <base_folder> <rel_binary_path> <binary_args>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <build> <build_system> <AFL_instrum> [options]" << std::endl;
        std::cout << "Usage: " << argv[0] << " <fuzz> <input_folder> <profile> [options]" << std::endl;
        std::cout << "Usage: " << argv[0] << " <coverage> [options] <output_folder1> [output_folder2] ..." << std::endl;
        std::cout << "Usage: " << argv[0] << " <kill>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <gather> [input_folder1] [input_folder2] ..." << std::endl;
        std::cout << "Usage: " << argv[0] << " <monitor> [fuzzing_path]" << std::endl;
        std::cout << "Usage: " << argv[0] << " <triage> [options] <crashes_folder1> [crashes_folder2] ..." << std::endl;
        std::cout << "Usage: " << argv[0] << " <copy> <input_folder> <output_folder> [options]" << std::endl;
        std::cout << "Usage: " << argv[0] << " <patterns> <output_folder1> [output_folder2] ..." << std::endl;
        std::cout << "Usage: " << argv[0] << " <break> <breakpoint> <crashes_folder1> [crashes_folder2] ..." << std::endl;
        std::cout << "Usage: " << argv[0] << " <plunger>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <telescope>" << std::endl;
        std::cout << std::endl;

    } else if (command == "setup") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <setup> <base_folder> <rel_binary_path> <binary_args>" << std::endl;

    } else if (command == "build") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <build> <build_system> <AFL_instrum> [options]" << std::endl;
        std::cout << "\n";
        std::cout << "\t build_system = {make | meson | cmake}" << std::endl;
        std::cout << "\t AFL_instrum = {gcc | llvm | lto}" << std::endl;
        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -c <ULI|afl|asan|cov>: partial compilation." << std::endl;
        std::cout << "\t -a <args>: optional arguments." << std::endl;
        std::cout << "\n";

    } else if (command == "fuzz") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <fuzz> <engine> [Options]" << std::endl;

        std::cout << "\n";
        std::cout << "\t engine = {AFL | ULI}" << std::endl;

        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -i <input_folder>: input folder. Default: input" << std::endl;
        std::cout << "\t -t <ms>: timeout for each execution. Default: 20ms" << std::endl;
        std::cout << "\t -s <bytes>: maximum size of the input." << std::endl;
        std::cout << "\t -n <num_cores>: number of cores to use. Default: 1" << std::endl;
        std::cout << "\t -e <extension>: extension of the input files." << std::endl;
        std::cout << "\t -d <dictionary>: custom dictionary" << std::endl;
        std::cout << "\n";

        std::cout << "\n";
        std::cout << "AFL options:" << std::endl;
        std::cout << "\t -p <profile>: profile to use. Mandatory" << std::endl;

        std::cout << "\n";
        std::cout << "ULIEngine options:" << std::endl;
        std::cout << "\t -c <mode>: coverage mode." << std::endl;
        std::cout << "\t -m <mutator>: enable mutator" << std::endl;

        std::cout << "" << std::endl;

    } else if (command == "coverage") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <coverage> [options] <output_folder1> [output_folder2] ..." << std::endl;
        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -n <num_threads>: number of threads to use. Default: 1" << std::endl;
        std::cout << "\n";

    } else if (command == "kill") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <kill>" << std::endl;

    } else if (command == "gather") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <gather> [input_folder1] [input_folder2] ..." << std::endl;

    } else if (command == "triage") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <triage> [options] <crashes_folder1> [crashes_folder2] ..." << std::endl;
        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -n <num_threads>: number of threads to use. Default: 1" << std::endl;
        std::cout << "\t -t <ms>: timeout for each execution. Default: Infinite" << std::endl;
        std::cout << "\t -r <num>: repeat the execution <num> times to catch non-deterministic crashes. Default: 5" << std::endl;
        std::cout << "\t -p <parser>: parser to use. Default: ASAN" << std::endl;
        std::cout << "\n";

    } else if (command == "copy") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <copy> <input_folder> <output_folder> [options]" << std::endl;
        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -l <bytes>: cap files to this size. Default: 1048576 bytes" << std::endl;
        std::cout << "\t -e <extension>: filter extension of input files. Default: no" << std::endl;
        std::cout << "\t -r: reverse order. Start copying from the end of the file. Default: no" << std::endl;
        std::cout << "\n";

    } else if (command == "patterns") {

        std::cout << std::endl;
        std::cout << "\tFind patterns in the crashes." << std::endl;
        std::cout << "\n";

        std::cout << "Usage: " << argv[0] << " <patterns> <output_folder1> [output_folder2] ..." << std::endl;
        std::cout << "\n";

    } else if (command == "break") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <break> <breakpoint> [options] <crashes_folder1> [crashes_folder2] ..." << std::endl;
        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -n <num_threads>: number of threads to use. Default: 1" << std::endl;
        std::cout << "\n";

    } else if (command == "install") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <install> <campaign> ..." << std::endl;

    } else if (command == "plunger") {

        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " <plunger>" << std::endl;
        std::cout << "\n";
        std::cout << "Options:" << std::endl;
        std::cout << "\t -y <yara_rules>: YARA rules file to use." << std::endl;
        std::cout << "\n";
        std::cout << "\n";

    } else {

        std::cerr << "Unknown command: " << command << std::endl;
    }
}

// void sleep_ms(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

int main(int argc, char *argv[]) {

    // test_parse_sanitizer_output();

    FRglobal ctx;

    //setenv("FRFUZZ_DEBUG_LEVEL", "1", 1);

    ctx.init();

    /*
    if (std::getenv("ULI_NO_UPDATE") == nullptr) {
        // auto_update();
    }
    */

    if (argc < 2) {
        print_help(argv);
        return 1;
    }

    std::string command(argv[1]);

    if (command != "list" && command != "install" && command != "build" && command != "fuzz" && command != "kill" && command != "gather" &&
        command != "monitor" && command != "triage" && command != "copy" && command != "patterns" && command != "break" && command != "tree" &&
        command != "coverage" && command != "plunger" && command != "telescope") {
        print_help(argv);
        return 1;
    }

    // Those options that don't require setup to be run

    if (command == "list") {

        autoconfig::list();

    } else if (command == "install") {

        if (argc < 3) {
            print_help(argv, "install");
            exit(EXIT_FAILURE);
        }

        using clock = std::chrono::steady_clock;
        auto t0 = clock::now();

        // Make std::cout a bit snappier for frequent flushes during spinner
        std::ios_base::sync_with_stdio(false);
        std::cout.tie(nullptr);

        std::string configuration(argv[2]);

        autoconfig::install(ctx, configuration);

        // Print current application path
        // std::cout << std::filesystem::current_path() << std::endl;

        auto t1 = clock::now();
        auto secs = duration_cast<std::chrono::seconds>(t1 - t0).count();
        std::cout << "⏱  Time elapsed: " << secs << " seconds \n" << std::endl;

        return 0;

    } else if (command == "tree") {

        if (argc < 3) {
            print_help(argv, "");
            exit(EXIT_FAILURE);
        }

        std::filesystem::path queue_folder(argv[2]);

        generate_dot(queue_folder);

        return 0;

    } else if (command == "monitor") {

        while (true) {

            if (argc > 2) {

                std::string AFL_path(argv[2]);
                monitor_info(AFL_path);
            } else {
                monitor_info();
            }

            sleep(15); // Run every 15 seconds
        }

    } else if (command == "kill") {

        fuzzer_kill();

    } else if (command == "gather") {

        if (argc < 3) {
            print_help(argv, "gather");
            exit(EXIT_FAILURE);
        }

        std::filesystem::path parent_folder = std::filesystem::current_path();
        std::string output_folder = "gather_" + std::filesystem::current_path().filename().string() + "_" + get_hostname();
        std::filesystem::path output_path = parent_folder / output_folder;

        std::vector<std::filesystem::path> input_folders;

        for (int i = 2; i < argc; i++) {
            input_folders.push_back(std::filesystem::path(argv[i]));
        }

        inputs_gather(parent_folder, output_folder, output_path, input_folders);

    } else if (command == "patterns") {

        if (argc < 3) {
            print_help(argv, "patterns");
            exit(EXIT_FAILURE);
        }

        std::vector<std::filesystem::path> input_files;

        for (int i = 2; i < argc; i++) {

            std::filesystem::path input_folder(argv[i]);
            if (!std::filesystem::exists(input_folder)) {
                std::cerr << "Folder " << input_folder << " does not exist" << std::endl;
                exit(EXIT_FAILURE);
            }

            for (auto &p : std::filesystem::recursive_directory_iterator(input_folder)) {

                if (p.is_regular_file()) {
                    input_files.push_back(p.path());
                }
            }
        }

        std::filesystem::path output_folder(argv[2]);

        exp_patterns(input_files, output_folder);

    } else if (command == "copy") {

        if (argc < 4) {
            print_help(argv, "copy");
            exit(EXIT_FAILURE);
        }

        std::filesystem::path input_folder(argv[2]);
        if (!std::filesystem::exists(input_folder)) {
            std::cerr << "Folder " << input_folder << " does not exist" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::filesystem::path output_folder(argv[3]);
        if (!std::filesystem::exists(output_folder)) {
            std::cerr << "Folder " << output_folder << " does not exist. Creating it..." << std::endl;
            std::filesystem::create_directories(output_folder);
        }

        size_t max_length = 1048576;
        std::string extension = "";
        bool reverse = false;

        int ch;
        while ((ch = getopt(argc, argv, "l:e:r")) != -1) {

            switch (ch) {

            case 'l': {
                max_length = std::stoi(optarg);
                break;
            }

            case 'e': {
                extension = optarg;
                if (extension[0] != '.') {
                    extension = "." + extension;
                }
                to_lower(extension);
                break;
            }

            case 'r': {
                reverse = true;
                break;
            }

            default:
                print_help(argv, "copy");
                exit(EXIT_FAILURE);
            }
        }

        copy_files(input_folder, output_folder, extension, max_length, reverse);

    } else {

        /*
        if (argc < 3) {
            print_help(argv);
            return 1;
        }
        */

        /*
        /home/$USER/.frfuzz
                /project
                        /version
                                /campaign
                                        /run1
                                        /run2
                                        /run3
        */

        std::filesystem::path campaign_folder = std::filesystem::current_path();

        Campaign *campaign = new Campaign();

        if (!campaign->load_from_disk(campaign_folder)) {
            std::cerr << "Error loading campaign from folder " << campaign_folder << std::endl;
            exit(EXIT_FAILURE);
        }

        ctx.campaign = campaign;

        // std::vector<std::filesystem::path> build_folders;

        if (command == "plunger") {

            if (argc < 2) {
                print_help(argv, "plunger");
                exit(EXIT_FAILURE);
            }

            // std::filesystem::path yara_rules_file = argv[4];

            std::filesystem::path output_folder;

            std::filesystem::path yara_rules_file;

            uint32_t interval = 60; // 60 seconds

            int ch;
            while ((ch = getopt(argc, argv, "i:y:")) != -1) {

                switch (ch) {

                case 'i': {
                    interval = std::stoi(optarg);
                    break;
                }

                case 'y': {
                    yara_rules_file = optarg;
                    break;
                }

                default:
                    print_help(argv, "build");
                    exit(EXIT_FAILURE);
                }
            }

            size_t arg_pos = 1 + optind;

            if (arg_pos == argc) {
                std::cerr << "Error: No output folder provided" << std::endl;
                print_help(argv, "plunger");
                exit(EXIT_FAILURE);
            }

            output_folder = argv[arg_pos];

            if (!std::filesystem::exists(output_folder)) {
                std::cerr << "Error: Output folder " << output_folder << " does not exist" << std::endl;
                exit(EXIT_FAILURE);
            }

            plunger(ctx, output_folder, yara_rules_file, interval);

            return 0;

        } else if (command == "build") {

            if (argc < 4) {
                print_help(argv, "build");
                exit(EXIT_FAILURE);
            }

            std::string build_system(argv[2]);

            std::string afl_instr(argv[3]);

            std::string compile = "all";
            std::string opt_args = "";

            int ch;
            while ((ch = getopt(argc, argv, "c:a:")) != -1) {

                switch (ch) {

                case 'c': {
                    compile = optarg;
                    break;
                }

                case 'a': {
                    opt_args = optarg;
                    break;
                }

                default:
                    print_help(argv, "build");
                    exit(EXIT_FAILURE);
                }
            }

            autobuild(build_system, afl_instr, compile, opt_args, ctx);

        } else if (command == "fuzz") {

            if (argc < 3) {
                print_help(argv, "fuzz");
                exit(EXIT_FAILURE);
            }

            std::string engine(argv[2]);

            if (engine != "AFL" && engine != "ULI") {
                std::cerr << "Invalid engine" << std::endl;
                std::cout << "Valid options are: AFL, ULI" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::filesystem::path output_path = campaign->unique_run_folder(engine);

            std::string input_path;

            size_t max_length = 0;

            size_t timeout = 20;

            std::vector<std::string> dictionary_paths;

            // dictionary_paths.push_back((std::filesystem::current_path() / "dict_pdf.txt").string());

            size_t cores = 1;

            // Global options
            int ch;
            while ((ch = getopt(argc, argv, "i:t:s:n:d:p:e:c:m:")) != -1) {

                switch (ch) {

                case 'i': {
                    input_path = optarg;
                    break;
                }

                case 't': {
                    timeout = std::stoi(optarg);
                    break;
                }

                case 's': {
                    max_length = std::stoi(optarg);
                    break;
                }

                case 'n': {
                    cores = std::stoi(optarg);
                    break;
                }

                case 'd': {
                    dictionary_paths.push_back(optarg);
                    break;
                }

                case 'p': {
                    // std::cout << "Option -p detected" << std::endl;
                    break;
                }

                case 'e': {
                    // std::cout << "Option -e detected" << std::endl;
                    break;
                }

                case 'm': {
                    // std::cout << "Option -m detected" << std::endl;
                    break;
                }

                case 'c': {
                    // std::cout << "Option -c detected" << std::endl;
                    break;
                }

                default:
                    print_help(argv, "fuzz");
                    exit(EXIT_FAILURE);
                }
            }

            // By default, use ./input as input folder
            if (input_path == "") {
                input_path = std::filesystem::current_path() / "input";
            }

            if (engine == "AFL") {

                std::cout << "Using AFL engine" << std::endl;

                const size_t cache_size = 600;

                std::string profileFile = "";

                std::string extension = "";

                size_t memory_limit = 4600;

                // AFL options
                optind = 1;
                int ch;
                while ((ch = getopt(argc, argv, "i:t:s:n:d:p:e:c:m:")) != -1) {

                    switch (ch) {

                    case 'p': {
                        profileFile = optarg;
                        break;
                    }

                    case 'e': {
                        extension = optarg;
                        break;
                    }

                    case 'i': {
                        break;
                    }

                    case 't': {
                        break;
                    }

                    case 's': {
                        break;
                    }

                    case 'n': {
                        break;
                    }

                    case 'd': {
                        break;
                    }

                    case 'm': {
                        break;
                    }

                    case 'c': {
                        break;
                    }

                    default:
                        print_help(argv, "fuzz");
                        exit(EXIT_FAILURE);
                    }
                }

                if (profileFile == "") {
                    print_help(argv, "fuzz");
                    exit(EXIT_FAILURE);
                }

                // If input folder does not exist, create it and add a seed file
                if (!std::filesystem::exists(input_path)) {
                    create_dir(input_path);
                    write_file(input_path + "/seed", "aaaa");
                }

                /*
                //Open profile file
                std::ifstream file(profileFile);

                //Check if file exists
                if (!file.is_open()) {
                    std::cout << "Error: Profile file does not exist" << std::endl;
                    return 1;
                }
                */

                fuzz_afl(profileFile, cores, input_path, output_path, max_length, timeout, memory_limit, extension, dictionary_paths, cache_size,
                         ctx);

            } else if (engine == "ULI") {

                std::string coverage_mode = "";
                std::vector<std::string> mutators_to_load;

                // ULI options
                optind = 1;
                int ch;

                while ((ch = getopt(argc, argv, "i:t:s:n:d:p:e:c:m:")) != -1) {

                    switch (ch) {

                    case 'c': {
                        coverage_mode = optarg;
                        break;
                    }

                    case 'm': {
                        mutators_to_load.push_back(optarg);
                        break;
                    }

                    case 'i': {
                        break;
                    }

                    case 't': {
                        break;
                    }

                    case 's': {
                        break;
                    }

                    case 'n': {
                        break;
                    }

                    case 'd': {
                        break;
                    }

                    case 'p': {
                        break;
                    }

                    case 'e': {
                        break;
                    }

                    default:
                        print_help(argv, "fuzz");
                        exit(EXIT_FAILURE);
                    }
                }

                std::cout << "Using ULI engine" << std::endl;

                launch_ULI(cores, input_path, output_path, coverage_mode, max_length, timeout, dictionary_paths, mutators_to_load);
            }

            // std::filesystem::path folder;
            // std::string name;
            // size_t process_number;

            // std::filesystem::path binary_path = folder / binary_rel_path;

            //-----------------------------------------------------------------------------------------
            //-----------------------------------------------------------------------------------------
        } else if (command == "triage") {

            if (argc < 3) {
                print_help(argv, "triage");
                exit(EXIT_FAILURE);
            }

            std::cout << "Optional arguments not provided. Trying to auto-detect the fuzzing setup..." << std::endl;
            std::cout << std::endl;

            std::vector<std::filesystem::path> crashes_folders;

            size_t timeout = 0; // Infinite

            // size_t numThreads = 1;

            size_t repeat = 5;
            // bool dumper = false;
            std::string parser = "ASAN";

            int ch;
            while ((ch = getopt(argc, argv, "t:n:r:p:")) != -1) {

                switch (ch) {

                case 't': {
                    timeout = std::stoi(optarg);
                    break;
                }

                case 'n': {
                    ctx.numThreads = std::stoi(optarg);
                    break;
                }

                case 'r': {
                    repeat = std::stoi(optarg);
                    break;
                }

                case 'p': {
                    parser = optarg;
                    break;
                }

                default:
                    print_help(argv, "triage");
                    exit(EXIT_FAILURE);
                }
            }

            size_t arg_pos = 1 + optind;

            if (arg_pos == argc) {
                std::cerr << "Error: No crashes folder provided" << std::endl;
                print_help(argv, "triage");
                exit(EXIT_FAILURE);
            }

            if (parser != "ASAN" && parser != "UBSAN" && parser != "GDB" && parser != "MALLOC") {
                std::cerr << "Error: Invalid parser" << std::endl;
                std::cerr << "Valid options are: ASAN, UBSAN, GDB, MALLOC" << std::endl;
                exit(EXIT_FAILURE);
            }

            for (int i = arg_pos; i < argc; i++) {
                std::filesystem::path folder(argv[i]);
                crashes_folders.push_back(folder);
            }

            triage(parser, crashes_folders, repeat, ctx);

        } else if (command == "break") {

            if (argc < 4) {
                print_help(argv, "break");
                exit(EXIT_FAILURE);
            }

            std::string breakpoint = argv[2];

            // size_t numThreads = 1;

            int ch;
            while ((ch = getopt(argc, argv, "n:")) != -1) {

                switch (ch) {

                case 'n': {
                    ctx.numThreads = std::stoi(optarg);
                    break;
                }

                default:
                    print_help(argv, "break");
                    exit(EXIT_FAILURE);
                }
            }

            size_t arg_pos = 2 + optind;

            if (arg_pos == argc) {
                std::cerr << "Error: No input folder provided" << std::endl;
                print_help(argv, "break");
                exit(EXIT_FAILURE);
            }

            std::vector<std::filesystem::path> output_folders;

            for (int i = arg_pos; i < argc; i++) {
                std::filesystem::path folder(argv[i]);
                output_folders.push_back(folder);
            }

            do_break(breakpoint, output_folders, ctx);

        } else if (command == "coverage") {

            // std::string outp = run("sleep infinity");

            if (argc < 3) {
                print_help(argv, "coverage");
                exit(EXIT_FAILURE);
            }

            // size_t numThreads = 1;

            int ch;
            while ((ch = getopt(argc, argv, "t:n:")) != -1) {

                switch (ch) {

                case 'n': {
                    ctx.numThreads = std::stoi(optarg);
                    break;
                }

                default:
                    print_help(argv, "coverage");
                    exit(EXIT_FAILURE);
                }
            }

            std::vector<std::filesystem::path> output_folders;

            // const std::filesystem::path output_folder(argv[3]);

            size_t arg_pos = 1 + optind;

            if (arg_pos == argc) {
                std::cerr << "Error: No output folder provided" << std::endl;
                std::cout << "arg_pos = " << arg_pos << std::endl;
                std::cout << "optind = " << optind << std::endl;
                print_help(argv, "coverage");
                exit(EXIT_FAILURE);
            }

            for (int i = arg_pos; i < argc; i++) {

                std::filesystem::path folder(argv[i]);
                output_folders.push_back(folder);
            }

            coverage(output_folders, ctx);
        }
    }
}
