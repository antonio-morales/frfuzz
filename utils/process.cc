/* SPDX-License-Identifier: AGPL-3.0-only */
#include "process.h"

std::vector<std::string> splitEnvs(const std::string &str) {

    std::vector<std::string> result;
    std::istringstream iss(str);

    std::string token;

    // Char by char iterate over the string
    for (int i = 0; i < str.size(); i++) {

        if (str[i] == '\"') {

            for (int j = i + 1; j < str.size(); j++) {

                if (str[j] == '\"') {
                    i = j;
                    result.push_back(token);
                    token = "";
                    break;
                } else {
                    token += str[j];
                }
            }

        } else if (str[i] == ' ' && token.size() > 0) {
            result.push_back(token);
            token = "";

        } else {
            token += str[i];
        }
    }

    if (token.size() > 0 && token != " ") {
        result.push_back(token);
    }

    return result;
}

std::string bash_escape(const std::string &s) {

    std::string result = "'";

    for (char c : s) {
        if (c == '\'') {
            result += "'\\''";
        } else {
            result += c;
        }
    }

    result += "'";

    return result;
}

int launch_terminal(int width, int height, int X, int Y, char const *cmd) {

    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s%s", cmd, "; exec bash");

    char geometry[256];
    std::string geometry_str =
        "--geometry=" + std::to_string(width) + "x" + std::to_string(height) + "+" + std::to_string(X) + "+" + std::to_string(Y);
    snprintf(geometry, sizeof(geometry), "%s", geometry_str.c_str());

    pid_t pid = 0;

    char *argv[] = {"gnome-terminal", geometry, "--", "bash", "-c", tmp, NULL};

    // char *argv[] = {"gnome-terminal", NULL};

    int status;

    status = posix_spawn(&pid, "/usr/bin/gnome-terminal", NULL, NULL, argv, environ);

    if (status == 0) {
        printf("Windows spawned: %s\n", geometry);
    } else {
        printf("poxis_spawn error: %s\n", strerror(status));
    }

    return pid;
}

int launch_process(char const *path, char *argv[]) {

    // char tmp[1024];
    // snprintf(tmp, sizeof(tmp), "%s%s", cmd, "; exec bash");

    // char geometry[256];
    // snprintf(geometry, sizeof(geometry), "%s%d%s%d", "--geometry=80x25+", X, "+", Y);

    pid_t child_pid = 0;

    // char *argv[] = {"gnome-terminal", geometry, "--", "bash" , "-c", tmp, NULL};

    int status;

    int ret;

    // const char *name = "GRFUZZ";
    // int anon_fd = memfd_create(name, NULL);

    posix_spawn_file_actions_t child_fd_actions;

    if (ret = posix_spawn_file_actions_init(&child_fd_actions))
        perror("posix_spawn_file_actions_init"), exit(ret);

    // STDOUT is redirected to /tmp/foo-log
    if (ret = posix_spawn_file_actions_addopen(&child_fd_actions, STDOUT_FILENO, "/tmp/foo-log", O_WRONLY | O_CREAT | O_TRUNC, 0644))
        perror("posix_spawn_file_actions_addopen"), exit(ret);

    // STDOUT is redirected to anonymous file
    // if (ret = posix_spawn_file_actions_adddup2 (&child_fd_actions, anon_fd, STDOUT_FILENO))
    // perror ("posix_spawn_file_actions_adddup2"), exit(ret);

    // STDERR is redirected to STDOUT
    if (ret = posix_spawn_file_actions_adddup2(&child_fd_actions, STDOUT_FILENO, STDERR_FILENO))
        perror("posix_spawn_file_actions_adddup2"), exit(ret);

    if (status = posix_spawnp(&child_pid, path, &child_fd_actions, NULL, argv, environ))
        perror("posix_spawn"), exit(ret);

    std::filesystem::path p(path);
    // std::cout << p.filename().string() << std::endl;

    char *arg = strdup(p.filename().string().c_str());

    // path = "/usr/bin/gnome-terminal"

    // status = posix_spawn(&pid, path, NULL, NULL, argv, environ);

    if (status == 0) {
        printf("Process spawned: \n");
    } else {
        printf("poxis_spawn error: %s\n", strerror(status));
    }

    std::string file_contents = read_file("/tmp/foo-log");

    std::cout << file_contents << std::endl;

    return child_pid;
}

// Example usage of execve and fork
void execute(char *argv[]) {

    char *envp[] = {NULL};

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvpe(argv[0], argv, envp);
        perror("execve");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            // printf("Child exited with status %d", WEXITSTATUS(status));
        }
    } else {
        perror("fork");
    }
}

std::string run(std::string command) {

    command += " 2>&1";

    const size_t BUFFER_SIZE = 4096;

    char buffer[BUFFER_SIZE];
    std::string result;

    // std::cout << "Command: " << command << std::endl;

    // std::cout << "Opening reading pipe" << std::endl;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Couldn't start command." << std::endl;
        std::cerr << "Command: " << command << std::endl;
        return "";
    }

    while (fgets(buffer, BUFFER_SIZE, pipe) != NULL) {
        // std::cout << "Reading..." << std::endl;
        result += buffer;
    }

    auto returnCode = pclose(pipe);

    // std::cout << result.length() << std::endl;
    //  std::cout << returnCode << std::endl;

    return result;
}

// timeout_ms = 0 means no timeout
std::string run(std::string command, size_t timeout_ms) {

    float seconds = timeout_ms / 1000.0;

    command = "timeout " + std::to_string(seconds) + " " + command;

    return run(command);
}

void run_thread(size_t thread_id, const std::vector<std::filesystem::path> &input_files, size_t posInicial, size_t posFinal,
                std::string partial_command, size_t timeout) {

    size_t num_elements = posFinal - posInicial + 1;

    bool has_at = false;

    size_t pos;

    std::string cmd_split1;
    std::string cmd_split2;

    if ((pos = partial_command.find("@@")) != std::string::npos) {
        has_at = true;
        cmd_split1 = partial_command.substr(0, pos);
        cmd_split2 = partial_command.substr(pos + 2);
    }

    for (int i = posInicial; i <= posFinal; i++) {

        std::string command;

        if (has_at) {

            // command = std::regex_replace(partial_command, std::regex("@@"), bash_escape(input_files[i].string()));

            command = cmd_split1 + bash_escape(input_files[i].string()) + cmd_split2;

        } else {

            command = partial_command + bash_escape(input_files[i].string());
        }

        if (thread_id == 0 && i % 1000 == 0) {
            debug() << "Current run: " << i - posInicial + 1 << " / " << num_elements << std::endl;
        }

        // std::cout << "Running command: " << command << std::endl;

        run(command, timeout);
    }
}

int launch_terminal(int width, int height, int X, int Y, std::string cmd, std::string custom_env) {

    // std::string str = "add string asdfasd\"this is a string with spaces!\"";
    // std::string str2 = "CC=afl-clang-lto AR=llvm-ar RANLIB=llvm-ranlib AS=llvm-as LD=afl-clang-lto CFLAGS=\"-Wno-error -g\"";
    // std::stringstream ss(str);
    // std::string word;

    std::vector<std::string> env = splitEnvs(custom_env);

    /*
    while (iss >> std::quoted(s)) {
        env.push_back(s);
    }
    */

    /*
    std::vector<std::string> env = split(custom_env, ' ');

    for(auto e : env) {
        if(e.back() == '"') {
            e.pop_back();
        }
    }
    */

    size_t environ_size = 0;
    for (char **env = environ; *env != 0; env++) {
        environ_size++;
    }

    size_t total_size = environ_size + env.size();

    char **environment = (char **)malloc((total_size + 1) * sizeof(char *));

    size_t i = 0;
    for (; i < environ_size; i++) {
        environment[i] = environ[i];
    }

    for (int j = 0; j < env.size(); j++) {
        environment[i] = strdup(env[j].c_str());
        i++;
    }

    environment[total_size] = NULL;

    // std::string tmp = cmd;
    std::string tmp = cmd + "; exec bash";
    char *command = strdup(tmp.c_str());

    char geometry[256];
    std::string geometry_str =
        "--geometry=" + std::to_string(width) + "x" + std::to_string(height) + "+" + std::to_string(X) + "+" + std::to_string(Y);
    snprintf(geometry, sizeof(geometry), "%s", geometry_str.c_str());

    pid_t pid = 0;

    char *argv[] = {"gnome-terminal", geometry, "--", "bash", "-c", command, NULL};

    // print command
    for (auto e : env) {
        std::cout << e << " ";
    }
    std::cout << command << std::endl;

    // char *argv[] = {"gnome-terminal", NULL};

    int status;

    status = posix_spawn(&pid, "/usr/bin/gnome-terminal", NULL, NULL, argv, environment);

    if (status == 0) {
        printf("Windows spawned: %s\n", geometry);
    } else {
        printf("poxis_spawn error: %s\n", strerror(status));
    }

    return pid;
}

int launch_shell(std::string cmd, std::string custom_env) {

    // std::string str = "add string asdfasd\"this is a string with spaces!\"";
    // std::string str2 = "CC=afl-clang-lto AR=llvm-ar RANLIB=llvm-ranlib AS=llvm-as LD=afl-clang-lto CFLAGS=\"-Wno-error -g\"";
    // std::stringstream ss(str);
    // std::string word;

    std::vector<std::string> env = splitEnvs(custom_env);

    /*
    while (iss >> std::quoted(s)) {
        env.push_back(s);
    }
    */

    /*
    std::vector<std::string> env = split(custom_env, ' ');

    for(auto e : env) {
        if(e.back() == '"') {
            e.pop_back();
        }
    }
    */

    size_t environ_size = 0;
    for (char **env = environ; *env != 0; env++) {
        environ_size++;
    }

    size_t total_size = environ_size + env.size();

    char **environment = (char **)malloc((total_size + 1) * sizeof(char *));

    size_t i = 0;
    for (; i < environ_size; i++) {
        environment[i] = environ[i];
    }

    for (int j = 0; j < env.size(); j++) {
        environment[i] = strdup(env[j].c_str());
        i++;
    }

    environment[total_size] = NULL;

    // std::string tmp = cmd;
    // std::string tmp = cmd + "; exec bash";
    char *command = strdup(cmd.c_str());

    // char geometry[256];
    // std::string geometry_str = "--geometry=" + std::to_string(width) + "x" + std::to_string(height) + "+" + std::to_string(X) + "+" +
    // std::to_string(Y); snprintf(geometry, sizeof(geometry), "%s", geometry_str.c_str());

    pid_t pid = 0;

    char *argv[] = {"bash", "-c", command, NULL};

    // print command
    for (auto e : env) {
        std::cout << e << " ";
    }
    std::cout << command << std::endl;

    // char *argv[] = {"gnome-terminal", NULL};

    posix_spawn_file_actions_t action;
    posix_spawn_file_actions_init(&action);
    posix_spawn_file_actions_addopen(&action, STDOUT_FILENO, "/dev/null", O_WRONLY | O_APPEND, 0);
    posix_spawn_file_actions_addopen(&action, STDERR_FILENO, "/dev/null", O_WRONLY | O_APPEND, 0);

    int status;

    status = posix_spawn(&pid, "/usr/bin/bash", &action, NULL, argv, environment);

    if (status == 0) {
        printf("Shell spawned: \n");
    } else {
        printf("poxis_spawn error: %s\n", strerror(status));
    }

    return pid;
}
