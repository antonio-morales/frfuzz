/* SPDX-License-Identifier: AGPL-3.0-only */
#include "plunger.h"

int plunger(FRglobal &ctx, std::filesystem::path output_folder, std::filesystem::path yara_rules_file, uint32_t interval) {

    // Check if there is an OPENAI_API_KEY in the secrets table
    std::vector<grDB::field_t> secrets = ctx.global_db->select_column("secrets", "value", "name='OPENAI_API_KEY'");

    std::string openai_key;

    std::string value = grDB::str(secrets[0]);

    if (value == "0") {

        std::cout << "OPENAI_API_KEY not found" << std::endl;

        std::cout << "Do you want to set it now? (y/n): ";
        char choice;
        std::cin >> choice;

        if (choice == 'y' || choice == 'Y') {

            std::cout << std::endl;
            std::cout << "    Enter your OpenAI API Key: ";
            std::cin >> openai_key;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear to end of line

            std::cout << std::endl;
            std::string password = get_password_masked("    Enter a password to encrypt your API Key: ");
            std::string password2 = get_password_masked("    Re-enter the password: ");

            if (password != password2) {
                std::cout << "    Passwords do not match. Exiting..." << std::endl;
                return 1;
            }

            EncBlob blob = encrypt_secret(password, openai_key);
            std::string packed = pack_blob(blob);

            ctx.global_db->update("secrets", "value", packed, "name='OPENAI_API_KEY'");

        } else {
            std::cout << "Exiting..." << std::endl;
            return 1;
        }

    } else {

        // Decrypt the API key
        EncBlob encrypted_blob = unpack_blob(value);

        std::string password = get_password_masked("    Enter the password to decrypt your API Key: ");
        std::string decrypted = decrypt_secret(password, encrypted_blob);
        openai_key = decrypted;
    }

    // Set timers

    int i = 0;

    // int interval = 15; // 15 seconds

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::cout << "\n[run " << ++i << "] starting at " << std::put_time(std::localtime(&t), "%H:%M:%S") << '\n';

    auto next = now + std::chrono::seconds(interval);
    std::time_t t_next = std::chrono::system_clock::to_time_t(next);
    std::cout << "[run " << i << "] next at " << std::put_time(std::localtime(&t_next), "%H:%M:%S") << '\n';

    YR_RULES *rules = NULL;

    if (yara_rules_file.string() != "") {
        std::cout << "Using YARA rules file: " << yara_rules_file << std::endl;

        if (yr_initialize() != ERROR_SUCCESS) {
            fprintf(stderr, "yr_initialize failed\n");
            return 1;
        }

        int rc = yc_load_rules(yara_rules_file.c_str(), &rules, NULL, 0);
        if (rc != ERROR_SUCCESS) {
            fprintf(stderr, "yc_load_rules failed: %d\n", rc);
            yr_finalize();
            return 1;
        }
    }

    while (1) {

        auto now_ms = [] { return std::chrono::steady_clock::now(); };
        auto ms = [](auto start, auto end) { return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); };

        auto t_start = now_ms();

        // Run coverage
        coverage(std::vector<std::filesystem::path>{output_folder}, ctx, false);

        // Parse LCOV

        std::filesystem::path tracefile_path = output_folder / "app2.info";
        tracefile_path = std::filesystem::canonical(tracefile_path);

        lcov::Tracefile tracefile(tracefile_path);

        if (!tracefile.parse()) {
            std::cerr << "Error parsing tracefile" << std::endl;
            return 1;
        }

        auto t_parse_done = now_ms();
        std::cerr << "\n[timing] parse took " << ms(t_start, t_parse_done) << " ms\n";

        // std::cout << tracefile << std::endl;

        std::vector<lcov::Function *> functions = tracefile.allFunctions();

        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(functions.begin(), functions.end(), rng);

        // Find functions with the specific pattern
        lcov::Function *target_function = nullptr;
        for (const auto &function : functions) {

            if (function->getExecutionCount() == 0) {
                continue; // Skip functions never executed
            }

            bool prev_was_covered = false;
            bool have_prev = false;
            bool pattern_found = false;

            int cu_transitions = 0;

            int i = 0;
            lcov::Line *line = nullptr;
            lcov::Line *prevLine = nullptr;
            for (i = function->getStartLine(); i <= function->getEndLine(); ++i) {
                line = function->getLine(i);
                if (!line->isLcovTrackedLine()) {
                    continue;
                }

                bool covered = line->getHits() > 0;

                if (have_prev) {
                    if (prev_was_covered && !covered) {

                        pattern_found = true;
                        cu_transitions++;

                        break;
                    }
                }

                // Update previous state
                prevLine = line;
                prev_was_covered = covered;
                have_prev = true;
            }

            if (pattern_found) {

                target_function = function;
                break;
            }
        }

        if (!target_function) {
            std::cerr << "No function with the specific pattern found." << std::endl;
            return 1;
        }

        std::string prompt_target_function = "";
        prompt_target_function += "Project: " + ctx.campaign->project_name() + " v" + ctx.campaign->project_version() + "\n";
        prompt_target_function += "File: " + target_function->getSourceFile()->getPath() + "\n";
        prompt_target_function += "Function: " + target_function->getName() + "\n";
        prompt_target_function += target_function->getCovText() + "\n";

        debug() << prompt_target_function << std::endl;

        // TODO: Functions callgraph
        // lcov::CallGraph graph(functions);
        // graph.build();
        // graph.printDot_coverage();
        // graph.printHTML_coverage();
        // graph.printMermaid_coverage();

        // TODO: Maybe it makes sense to read the instructions and content from a text file

        std::string instructions = "You are a fuzzing input file generator."
                                   "Your sole job: produce exactly one candidate input file to exercise uncovered code.";

        std::string content = "I don't have coverage of the following function:\n\n";

        content += prompt_target_function;

        content += "\n\n";

        content += "Goal: Provide ONE input file that would cause the maximum function uncovered lines to execute."
                   "Just return the file content, no explanations."
                   "If the file format is text, return the plain text."
                   "If the file format is binary, return the content as an hex string where each byte is prefixed with '0x'\n";

        auto t_before_http = now_ms();

        llm::Client cl("gpt-5-mini", openai_key);

        cl.setInstructions(instructions);

        cl.setEffort("low");

        std::string resp = cl.send(content);

        if (!resp.empty()) {
            std::cout << "\nResponse from LLM:\n";
            std::cout << resp << "\n";
        } else {
            std::cout << "No output_text found in response.\n";
            continue;
        }

        auto t_after_http = now_ms();
        std::cout << "\n[timing] HTTP request: " << ms(t_before_http, t_after_http) << " ms\n";

        // unsigned char *buf1 = resp.c_str();

        std::string decoded;

        if (is_hex_string(resp)) {
            std::cout << "Response is hex encoded." << std::endl;
            decoded = hex_decode(resp);
        } else {
            std::cout << "Response is plain text." << std::endl;
            decoded = resp;
        }

        if (rules) {
            std::cout << "Scanning response with YARA rules..." << std::endl;

            int match = 0;

            const uint8_t *buf = reinterpret_cast<const uint8_t *>(decoded.data());

            int rc = yr_rules_scan_mem(rules, buf, decoded.size(), 0, _match_cb, &match, 0);

            if (rc != ERROR_SUCCESS) {
                fprintf(stderr, "yr_rules_scan_mem failed: %d\n", rc);
            } else {
                printf("Match? %s\n", match ? "yes" : "no");
            }
        }

        std::string new_input_filename =
            "plunger_" +
            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        // TODO: We need to refine this and consider other instance names
        std::filesystem::path new_input_path = output_folder / "MASTER_LLVM" / "queue";
        new_input_path = std::filesystem::canonical(new_input_path);

        write_file(new_input_path / new_input_filename, decoded);

        if (std::chrono::system_clock::now() < next) {
            std::this_thread::sleep_until(next);
        }

        next += std::chrono::seconds(interval);
    }

    yr_finalize();

    // printf("Hello, World!\n");

    return 0;
}