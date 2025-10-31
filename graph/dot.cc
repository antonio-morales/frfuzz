/* SPDX-License-Identifier: AGPL-3.0-only */
#include "dot.h"

int testcase_sort(const struct dirent **a, const struct dirent **b) {

    const char *name1 = (*a)->d_name;
    const char *name2 = (*b)->d_name;

    // Handle . and ..
    if (name1[0] == '.') {
        return 1;
    }

    if (name2[0] == '.') {
        return -1;
    }

    char num1[9];
    memcpy(num1, name1, 8);
    num1[8] = '\0';

    char num2[9];
    memcpy(num2, name2, 8);
    num2[8] = '\0';

    uint32_t n1 = strtol(num1, nullptr, 10);
    uint32_t n2 = strtol(num2, nullptr, 10);

    return n1 - n2;
}

bool generate_dot(std::filesystem::path queue_folder) {

    struct dirent **namelist;

    int num_files = scandir(queue_folder.c_str(), &namelist, NULL, testcase_sort);

    if (num_files < 0) {
        std::cerr << "Error reading the queue folder" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (num_files == 0) {
        std::cerr << "No testcases found in the queue folder" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<node> nodes(num_files);

    nodes[0].depth = 0;

    uint32_t tree_depth = 0;

    for (int i = 0; i < num_files; i++) {

        char *filename = namelist[i]->d_name;

        if (filename[0] == '.') {
            continue;
        }

        node n = testcase_to_node(std::string(filename));

        if (n.id < 0) {
            std::cerr << "Error parsing testcase filename: " << filename << std::endl;
            continue;
        }

        nodes[n.id] = n;

        if (n.parent_id >= 0) {
            nodes[n.parent_id].children.push_back(n.id);
            nodes[n.id].depth = nodes[n.parent_id].depth + 1;

            if (nodes[n.id].depth > tree_depth) {
                tree_depth = nodes[n.id].depth;
            }
        }
    }

    // Generate a DOT file with the tree

    std::filesystem::path dot_path = queue_folder.parent_path() / "tree.dot";

    std::ofstream dot_file(dot_path);

    if (!dot_file.is_open()) {
        std::cerr << "Error opening the dot file" << std::endl;
        exit(EXIT_FAILURE);
    }

    dot_file << "digraph tree {" << std::endl;

    for (const auto &n : nodes) {

        dot_file << n.id << " -> " << n.parent_id << ";" << std::endl;
    }

    dot_file << "}" << std::endl;

    dot_file.close();

    // dot -Tsvg tree.dot -o tree.svg

    // Sort the nodes by the number of children

    std::vector<node> sorted_nodes = nodes;

    std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](const node &a, const node &b) { return a.children.size() > b.children.size(); });

    std::cout << "Tree depth: " << tree_depth << std::endl;

    // TODO:Check in the configure that we have all the needed libaries/tools

    if (system("command -v xxd") != 0) {
        std::cerr << "xxd is not installed. Skipping diff." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (system("command -v colordiff") != 0) {
        std::cerr << "colordiff is not installed. Skipping diff." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (const auto &n : sorted_nodes) {

        std::cout << n.id << " (" << n.children.size() << " children) -> " << n.parent_id << "[level " << n.depth << "]" << std::endl;

        // Show the file diff between the node and the parent

        if (n.parent_id >= 0) {

            std::string out1 = run("xxd " + queue_folder.string() + "/" + std::string(n.filename));
            write_file("/tmp/file1.hex", out1);

            std::string out2 = run("xxd " + queue_folder.string() + "/" + std::string(nodes[n.parent_id].filename));
            write_file("/tmp/file2.hex", out2);

            std::string cmd = "colordiff -y /tmp/file1.hex /tmp/file2.hex";

            std::string output = run(cmd);

            std::cout << output << std::endl;
        }
    }

    return true;
}