/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

// pid_t library
#include <sys/types.h>

// fork library
#include <unistd.h>

// perror library
#include <stdio.h>

// exit library
#include <stdlib.h>

// waitpid library
#include <sys/wait.h>

#include <spawn.h>
#include <string.h>

#include <fcntl.h>

#include <chrono>
#include <filesystem>
#include <vector>

#include "utils/debug.h"
#include "utils/filesys.h"

int launch_terminal(int width, int height, int X, int Y, char const *cmd);
int launch_terminal(int width, int height, int X, int Y, std::string cmd, std::string custom_env);
int launch_shell(std::string cmd, std::string custom_env);

std::string bash_escape(const std::string &s);

void execute(char *argv[]);

std::string run(std::string command);

std::string run(std::string command, size_t timeout_ms);

void run_thread(size_t thread_id, const std::vector<std::filesystem::path> &input_files, size_t posInicial, size_t posFinal,
                std::string partial_command, size_t timeout);

class process {

  private:
    pid_t pid;
    int status;

    std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

    std::vector<std::string> argv;

    std::vector<std::string> envp;

  protected:
    std::string executable_path;

  public:
    process() {}

    ~process() {}

    inline std::chrono::time_point<std::chrono::system_clock> get_start_time() const { return start_time; };

    bool spawn() { start_time = std::chrono::system_clock::now(); };
};
