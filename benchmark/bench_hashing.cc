/* SPDX-License-Identifier: AGPL-3.0-only */
void bench_git_hash_object() {

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < UINT16_MAX * 20; i++) {

        git_hash_object("/home/...");
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Time elapsed: " << duration.count() << " milliseconds" << std::endl;
}

void bench_is_executable_file() {

    std::filesystem::path path("/home/...");

    int j = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000; i++) {
        if (is_executable_file(path)) {
            j++;
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Time elapsed: " << duration.count() << " milliseconds" << std::endl;

    std::cout << j << std::endl;

    exit(0);
}
