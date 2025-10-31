/* SPDX-License-Identifier: AGPL-3.0-only */
#include "monitor.h"

void monitor_info(std::string AFL_dir) {

    // Clear terminal screen
    std::cout << "\033[2J\033[1;1H";

    std::cout << "---> Monitoring..." << std::endl;

    std::string out_str = run("apcaccess");

    // Get line by line

    std::istringstream f(out_str);
    std::string line;

    while (std::getline(f, line)) {

        if (line.starts_with("HOSTNAME")) {
            std::cout << line << std::endl;

        } else if (line.starts_with("STATUS")) {
            std::cout << line << std::endl;

            if (line.find("ONBATT") != std::string::npos) {
                std::cout << "UPS on battery. Suspending." << std::endl;

                // Alert other computers via ssh

                // run("systemctl suspend");
            }

        } else if (line.starts_with("BCHARGE")) {
            std::cout << line << std::endl;
        }
    }

    std::cout << std::endl;

    out_str = run("sensors");

    std::istringstream g(out_str);

    while (std::getline(g, line)) {

        if (line.starts_with("Tctl")) {
            std::cout << line << std::endl;

        } else if (line.starts_with("Tccd")) {
            std::cout << line << std::endl;

        } else if (line.starts_with("Package id")) {
            std::cout << line << std::endl;
        }
    }

    std::cout << std::endl;

    if (AFL_dir.size() > 0) {

        std::string cmd = "afl-whatsup -s " + AFL_dir;
        out_str = run(cmd);
        std::cout << out_str << std::endl;
    }
}