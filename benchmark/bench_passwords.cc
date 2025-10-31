/* SPDX-License-Identifier: AGPL-3.0-only */
void bench_generate_password() {

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000000; i++) {

        std::string test = random_password(25);
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "Time elapsed: " << duration.count() << " milliseconds" << std::endl;
}