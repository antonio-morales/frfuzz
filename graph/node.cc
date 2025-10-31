/* SPDX-License-Identifier: AGPL-3.0-only */
#include "node.h"

node testcase_to_node(std::string testcase_filename) {

    node n;

    const std::string orig_filename = testcase_filename;

    if (testcase_filename.starts_with("id:") == false) {
        return node();
    }

    testcase_filename = testcase_filename.substr(3); // Remove "id:"

    // Search the , which indicates the end of the ID
    size_t comma_pos = testcase_filename.find(',');
    if (comma_pos == std::string::npos) {
        return node();
    }

    std::string id_str = testcase_filename.substr(0, comma_pos);
    n.id = std::stoul(id_str, nullptr, 10);

    n.filename = strdup(orig_filename.c_str());

    if (n.id == 0) {
        n.depth = 0;
        return n;
    }

    // Remove up to the comma_pos
    testcase_filename = testcase_filename.substr(comma_pos + 1);

    if (testcase_filename.starts_with("src:") == false) {
        return node();
    }

    testcase_filename = testcase_filename.substr(4); // Remove "src:"

    // Search the , which indicates the end of the parent_ID
    comma_pos = testcase_filename.find(',');
    if (comma_pos == std::string::npos) {
        return node();
    }

    std::string parent_id_cstr = testcase_filename.substr(0, comma_pos);
    n.parent_id = std::stoul(parent_id_cstr, nullptr, 10);

    return n;
}

/*

size_t filename_length = testcase_filename.length();

if(filename_length < id_length){
    return n;
}

char node_id_cstr[id_length + 1];
memcpy(node_id_cstr, testcase_filename.c_str(), id_length);
node_id_cstr[id_length] = '\0';

n.id = std::stoul(node_id_cstr, nullptr, 10);



n.filename = strdup(testcase_filename.c_str());

if(filename_length < (id_length * 2 + 2)){
    return n;
}

char parent_id_cstr[ID_LENGTH + 1];
memcpy(parent_id_cstr, testcase_filename + ID_LENGTH + 2, ID_LENGTH);
parent_id_cstr[ID_LENGTH] = '\0';

n.parent_id = std::stoul(parent_id_cstr, nullptr, 10);
*/