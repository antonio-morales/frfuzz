/* SPDX-License-Identifier: AGPL-3.0-only */
#include "autobuild.h"

std::vector<std::string> tokenize(const std::string &str) {

    std::vector<std::string> tokens;

    std::istringstream iss(str);
    for (std::string s; iss >> s;) {
        tokens.push_back(s);
    }

    return tokens;
}

void autobuild(std::string build_system, std::string afl_instr, std::string compile, std::string opt_args, const FRglobal &ctx) {

    int Pos_X = 50;
    int Pos_Y = 50;

    const int width = 85;
    const int height = 27;

    std::string command;

    if (build_system == "make") {

        bool autogen = false;
        bool configure = false;

        if (std::filesystem::exists(ctx.campaign->src_folder / "configure")) {
            configure = true;

        } else {

            if (std::filesystem::exists(ctx.campaign->src_folder / "autogen.sh")) {
                autogen = true;
                configure = true;
            }
        }

        if (autogen) {
            command += "./autogen.sh; ";
        }

        if (configure) {
            command += "./configure --prefix=FRFUZZ_INSTALL_DIR --disable-shared; ";
        }

        command += "make -j 4";

        if (configure) {
            command += "; make install";
        }

    } else if (build_system == "meson") {

        //-Ddefault_library=static
        command += "meson setup ";
        command += "AFL_build; ninja -vC AFL_build";

    } else if (build_system == "cmake") {

        command += "mkdir bin; cd bin; cmake ..; make -j 4";

    } else {
        std::cerr << "Invalid build system" << std::endl;
        std::cout << "Valid options are: make, meson, cmake" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string build_env;

    if (afl_instr == "gcc") {

        build_env = "CC=afl-gcc CXX=afl-g++ ";

    } else if (afl_instr == "llvm") {

        build_env = "CC=afl-clang-fast CXX=afl-clang-fast++ ";

    } else if (afl_instr == "lto") {

        if (build_system == "make" || build_system == "cmake") {

            build_env = "CC=afl-clang-lto CXX=afl-clang-lto++ AR=llvm-ar RANLIB=llvm-ranlib AS=llvm-as LD=afl-ld ";

        } else if (build_system == "meson") {

            build_env += "CC=afl-clang-lto CXX=afl-clang-lto++ LDFLAGS=-fuse-ld=afl-ld ";
        }

    } else {
        std::cerr << "Invalid AFL_INSTR" << std::endl;
        std::cout << "Valid options are: gcc, llvm, lto" << std::endl;
        exit(EXIT_FAILURE);
    }

    // const std::string build_env_LTO = "CC=" + build_compiler + " CXX=" + build_compiler + "++ AR=llvm-ar RANLIB=llvm-ranlib AS=llvm-as
    // LD=afl-clang-lto";

    // const std::string build_env_LLVM = "CC=afl-clang-fast CXX=afl-clang-fast++ AR=llvm-ar RANLIB=llvm-ranlib AS=llvm-as LD=afl-clang-fast";

    // std::string command = "./configure --disable-shared";

    std::vector<std::string> to_build;

    if (compile == "cov") {
        to_build = {"__COV"};

    } else if (compile == "ULI") {
        to_build = {"__ULI"};

    } else if (compile == "afl") {
        to_build = {"__AFL"};

    } else if (compile == "asan") {
        to_build = {"__ASAN"};

    } else if (compile == "asan_noopt") {
        to_build = {"__ASAN_NOOPT"};

    } else if (compile == "ubsan") {
        to_build = {"__UBSAN"};

    } else if (compile == "afl_asan") {
        to_build = {"__AFL_ASAN"};

    } else if (compile == "afl_ubsan") {
        to_build = {"__AFL_UBSAN"};

    } else if (compile == "afl_ctx") {
        to_build = {"__AFL_CTX"};

    } else if (compile == "afl_caller") {
        to_build = {"__AFL_CALLER"};

    } else if (compile == "afl_ngram") {
        to_build = {"__AFL_NGRAM"};

    } else if (compile == "afl_ngram_asan") {
        to_build = {"__AFL_NGRAM_ASAN"};

    } else if (compile == "cov") {
        to_build = {"__COV"};

    } else if (compile == "gprof") {
        to_build = {"__GPROF"};

    } else if (compile == "all") {
        to_build = afl_builds;

    } else {
        std::cerr << "Invalid option " << compile
                  << ". Valid options are: ULI, afl, asan, asan_noopt, ubsan, afl_asan, afl_ubsan, afl_ctx, afl_caller, afl_ngram, afl_ngram_asan, "
                     "cov, gprof"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    if (opt_args != "") {

        // Create an array of the words of opt_args
        std::vector<std::string> tokenized_args = tokenize(opt_args);

        if (build_system == "meson" || build_system == "cmake") {

            // Iterate over the array and delete those that not start with "-D"
            for (auto it = tokenized_args.begin(); it != tokenized_args.end();) {
                if (it->find("-D") == std::string::npos && it->find("--") == std::string::npos) {
                    it = tokenized_args.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Join the array into a string
        std::string str_params;
        for (auto it = tokenized_args.begin(); it != tokenized_args.end(); ++it) {
            str_params += *it + " ";
        }

        if (build_system == "meson") {

            command = std::regex_replace(command, std::regex("meson setup"), "meson setup " + str_params);

        } else if (build_system == "cmake") {

            command = std::regex_replace(command, std::regex("cmake"), "cmake " + str_params);
        }
    }

    for (auto build : to_build) {

        std::filesystem::path new_filename = ctx.campaign->src_folder.filename().string() + build;

        if (build != "__ULI" && build != "__COV" && build != "__GPROF" && build != "__ASAN" && build != "__ASAN_NOOPT" && build != "__UBSAN") {
            new_filename += "_" + afl_instr;
        }

        std::filesystem::path new_folder = ctx.campaign->campaign_path / new_filename;

        // build_folders.push_back(new_folder);

        std::string cmd = "cp -r -p ";
        cmd = cmd + ctx.campaign->src_folder.c_str() + " " + new_folder.c_str();
        system(cmd.c_str());

        std::string autodict_env = "";
        if (afl_instr == "llvm") {
            autodict_env = "AFL_LLVM_DICT2FILE=\"" + new_folder.string() + "/AUTODICT.txt\" AFL_LLVM_DICT2FILE_NO_MAIN=1 ";
        }

        std::string env = "";
        if (build_system == "meson") {

        } else {
            env += "CFLAGS=\"-Wno-error -g\" CXXFLAGS=\"-Wno-error -g\" ";
        }

        std::string new_command = command;
        std::string new_build = build_env;

        if (build == "__ULI") {

            if (afl_instr != "llvm") {
                std::cerr << "ULI only works with LLVM" << std::endl;
                exit(EXIT_FAILURE);
            }

            new_build = std::regex_replace(new_build, std::regex("CC=afl-clang-fast CXX=afl-clang-fast++"), "CC=ULICompiler CXX=ULICompiler");

            new_command = std::regex_replace(new_command, std::regex("FRFUZZ_INSTALL_DIR"), new_folder.string() + "/ULIINSTALL");

        } else if (build == "__AFL") {

            new_command = std::regex_replace(new_command, std::regex("FRFUZZ_INSTALL_DIR"), new_folder.string() + "/ULIINSTALL");

        } else if (build == "__AFL_ASAN") {

            if (build_system == "meson") {
                new_command = std::regex_replace(new_command, std::regex("meson setup"), "meson setup -Db_sanitize=address -Db_lundef=false");

            } else if (build_system == "cmake") {
                new_command =
                    std::regex_replace(new_command, std::regex("cmake"),
                                       "cmake -DCMAKE_C_FLAGS=\"-fsanitize=address\" -DCMAKE_CXX_FLAGS=\"-fsanitize=address\" "
                                       "-DCMAKE_EXE_LINKER_FLAGS=\"-fsanitize=address\" -DCMAKE_SHARED_LINKER_FLAGS=\"-fsanitize=address\"");

            } else {
                env += "AFL_USE_ASAN=1 ";
            }

        } else if (build == "__AFL_UBSAN") {

            if (build_system == "meson") {
                new_command = std::regex_replace(new_command, std::regex("meson setup"), "meson setup -Db_sanitize=undefined -Db_lundef=false");

            } else if (build_system == "cmake") {
                new_command =
                    std::regex_replace(new_command, std::regex("cmake"),
                                       "cmake -DCMAKE_C_FLAGS=\"-fsanitize=undefined\" -DCMAKE_CXX_FLAGS=\"-fsanitize=undefined\" "
                                       "-DCMAKE_EXE_LINKER_FLAGS=\"-fsanitize=undefined\" -DCMAKE_SHARED_LINKER_FLAGS=\"-fsanitize=undefined\"");

            } else {
                env += "AFL_USE_UBSAN=1 ";
            }

        } else if (build == "__AFL_CFISAN") {

            env += "AFL_USE_CFISAN=1 ";

        } else if (build == "__AFL_COMPCOV") {

            env += "AFL_LLVM_LAF_ALL=1 ";

        } else if (build == "__AFL_CTX") {

            env += "AFL_LLVM_CTX=1 ";

        } else if (build == "__AFL_CALLER") {

            env += "AFL_LLVM_CALLER=1 ";

        } else if (build == "__AFL_NGRAM") {

            env += "AFL_LLVM_NGRAM_SIZE=6 ";

        } else if (build == "__ASAN" || build == "__ASAN_NOOPT" || build == "__UBSAN" || build == "__AFL_NGRAM_ASAN") {

            if (build == "__AFL_NGRAM_ASAN") {
                env += "AFL_LLVM_NGRAM_SIZE=6 ";
            }

            std::string SAN;

            if (build == "__ASAN" || build == "__ASAN_NOOPT" || build == "__AFL_NGRAM_ASAN") {
                SAN = "address";
            } else {
                SAN = "undefined";
            }

            if (build != "__AFL_NGRAM_ASAN") {
                new_build = std::regex_replace(new_build, std::regex("AR=llvm-ar RANLIB=llvm-ranlib AS=llvm-as LD=afl-ld "), "");
                new_build = std::regex_replace(new_build, std::regex("afl-gcc"), "gcc");
                new_build = std::regex_replace(new_build, std::regex("afl-clang-fast"), "clang");
                new_build = std::regex_replace(new_build, std::regex("afl-clang-lto"), "clang");
            }

            if (build_system == "meson") {

                std::string subst_str = "meson setup -Db_sanitize=" + SAN + " -Db_lundef=false";

                if (build == "__ASAN_NOOPT") {
                    subst_str += " -Dc_args=\"-fsanitize-recover=address\" --buildtype=debug";
                }

                new_command = std::regex_replace(new_command, std::regex("meson setup"), subst_str);

            } else if (build_system == "cmake") {
                std::string subst_str = "cmake -DCMAKE_C_FLAGS=\"-fsanitize=" + SAN + "\" -DCMAKE_CXX_FLAGS=\"-fsanitize=" + SAN +
                                        "\" -DCMAKE_EXE_LINKER_FLAGS=\"-fsanitize=" + SAN + "\" -DCMAKE_SHARED_LINKER_FLAGS=\"-fsanitize=" + SAN +
                                        "\"";

                if (build == "__ASAN_NOOPT") {
                    subst_str += " -DCMAKE_BUILD_TYPE=Debug";
                }

                new_command = std::regex_replace(new_command, std::regex("cmake"), subst_str);

            } else {
                std::string subst_str = "-Wno-error -g -fsanitize=" + SAN;

                if (build == "__ASAN_NOOPT") {
                    subst_str += " -O0";
                }

                env = std::regex_replace(env, std::regex("-Wno-error -g"), subst_str);
                env += "LDFLAGS=\"-fsanitize=" + SAN + "\" ";
            }

        } else if (build == "__COV") {

            new_build = "CC=gcc CXX=g++ ";

            if (build_system == "meson") {

                new_command = std::regex_replace(new_command, std::regex("meson setup"), "meson setup --buildtype=debug -Db_coverage=true");

            } else {

                new_command = std::regex_replace(new_command, std::regex("FRFUZZ_INSTALL_DIR"), new_folder.string() + "/ULIINSTALL");

                env = std::regex_replace(env, std::regex("-Wno-error -g"), "-Wno-error -g --coverage");
                env += "LDFLAGS=\"--coverage\" ";
            }

        } else if (build == "__GPROF") {

            new_build = "CC=gcc CXX=g++ ";

            if (build_system == "meson") {

                new_command = std::regex_replace(new_command, std::regex("meson setup"),
                                                 "meson setup --buildtype=debug -Dc_args=\"-pg\" -Dc_link_args=\"-pg\"");

            } else {

                env = std::regex_replace(env, std::regex("-Wno-error -g"), "-Wno-error -g -pg");
            }
        }

        Pos_X += 50;
        Pos_Y += 50;

        env = autodict_env + new_build + env;

        // std::cout << env << std::endl;
        // exit(0);

        cmd = "cd " + new_folder.string() + "; " + new_command;

        int pid = launch_terminal(width, height, Pos_X, Pos_Y, cmd.c_str(), env);
    }

    /*
    for (auto folder : build_folders) {
    // std::filesystem::copy(folder, new_folder, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    std::string cmd = "cp -r -p ";
    cmd = cmd + ctx.src_folder.c_str() + " " + folder.c_str();
    system(cmd.c_str());
    }
    */
}