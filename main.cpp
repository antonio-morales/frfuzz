/* SPDX-License-Identifier: AGPL-3.0-only */

#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstring>

#include <sstream>

#include <nlohmann/json.hpp>

#include "data.h"
#include "output.h"
// #include "utils.h"
// #include "filesys.h"
#include "GitHub/API.h"
#include "coverage/coverage.h"
#include "dump.h"
#include "progress.h"
#include "thread.h"

std::vector<std::string> *gather_LCOV_files(std::string path) {

    std::vector<std::string> *files = new std::vector<std::string>();

    if (!std::filesystem::is_directory(path)) {
        std::cout << "Error: " << path << " is not a directory" << std::endl;
        return NULL;
    }

    // Use recursive_directory_iterator
    for (auto &p : std::filesystem::recursive_directory_iterator(path)) {
        if (p.path().extension() == ".html") {

            // We only want to parse LCOV individual files, not report files
            // So we look for those files contains "source-name-title" string

            std::ifstream input_file(p.path());
            std::string line;
            while (std::getline(input_file, line)) {
                if (line.find("source-name-title") != std::string::npos) {
                    files->push_back(p.path());
                    break;
                }
            }
        }
    }

    return files;
}

// Class template bidirectional tree
class CallGraph {

  public:
    CallGraph(std::multimap<LCOVfunction, LCOVfunction> map) {

        // std::unordered_set<LCOVfunction, LCOVfunction::HashFunction> functions;

        std::unordered_map<LCOVfunction, uint32_t, LCOVfunction::HashFunction> functions;

        for (const auto &it : map) {

            LCOVfunction f = it.first;
            auto ret = functions.insert({f, 0});

            uint32_t parentID, childID;

            // If function does not exist, insert a new node
            if (ret.second) {

                Node n(f);
                parentID = this->insert(n);

                functions[f] = parentID; // Update node ID
            } else {
                parentID = ret.first->second;
            }

            f = it.second;
            ret = functions.insert({f, 0});
            if (ret.second) {

                Node n(f);
                childID = this->insert(n);
                functions[f] = childID;
            } else {
                childID = ret.first->second;
            }

            children_nodes.insert({parentID, childID});
        }

        /*
        std::cout << "Total nodes: " << this->nodes.size() << std::endl;
        for(const auto &it: child_nodes){

            std::cout << it.first << " --> " << it.second << std::endl;

            std::cout << nodes[it.first].data.getName() << " --> " << nodes[it.second].data.getName() << std::endl;

            std::cout << std::endl;

            //Node n(it, this);

            //this->insert(n);

            //nodes.insert(n);
        }
        */

        // parent_nodes map
        for (const auto &it : this->children_nodes) {

            uint32_t parentID = it.first;

            uint32_t childID = it.second;

            parent_nodes.insert({it.second, it.first});

            // this->parent_nodes.insert({it.second, it.first});
        }

        // Find those parents that are not children
        // This will be the root nodes
        for (const auto &it : this->children_nodes) {

            uint32_t parentID = it.first;

            if (parent_nodes.count(parentID) == 0) {
                root_nodes.insert(parentID);
            }
        }

        for (const auto &it : this->root_nodes) {
            std::cout << "Root node: " << nodes[it].data.getName() << std::endl;
        }

        // Find those children that are not parents
        // This will be the leaf nodes
        for (const auto &it : this->parent_nodes) {

            uint32_t childID = it.first;

            if (children_nodes.count(childID) == 0) {
                leaf_nodes.insert(childID);
            }
        }

        for (const auto &it : this->leaf_nodes) {
            std::cout << "Leaf node: " << nodes[it].data.getName() << std::endl;
        }
    }

    std::vector<std::vector<uint32_t>> execPaths() {

        if (!execPathsComputed) {
            prv__executionPaths();
            execPathsComputed = true;
        }

        return prv__execPaths;
    }

    void longestPath() {

        // Get the longest execution path
        std::vector<uint32_t> longestPath;

        for (const auto &it : execPaths()) {

            if (it.size() > longestPath.size()) {
                longestPath = it;
            }
        }

        uint32_t treeHeight = longestPath.size();

        std::cout << "\nLongest execution path: " << treeHeight << std::endl;

        std::cout << printPath(longestPath, "text") << std::endl;
    }

    std::multimap<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> sortPaths(uint32_t (LCOVfunction::*sortMethod)() const) {

        uint32_t pathsNum = execPaths().size();

        std::vector<uint32_t> execPathsCov(pathsNum, 0);

        int i = 0;
        for (auto &it : execPaths()) {

            for (auto &n : it) {
                Node &node = this->getNodeRef(n);

                LCOVfunction *obj = &node.data;

                uint32_t score = (obj->*sortMethod)();

                execPathsCov[i] += score;
            }

            i++;
        }

        std::multimap<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> execPathsCovMap;

        for (int i = 0; i < pathsNum; i++) {

            auto ret = execPathsCovMap.insert({execPathsCov[i], std::vector<uint32_t>()});

            ret->second = prv__execPaths[i];
        }

        for (const auto &it : execPathsCovMap) {

            std::cout << "\nCoverage: " << it.first << std::endl;

            std::cout << printPath(it.second, "text") << std::endl;
        }

        // Mediante esta estrategia hemos sacado el nodo raiz más principal (ares_getnameinfo), que es aquel que puede alcanzar los execution paths
        // con mayor coverage

        return execPathsCovMap;
    }

    // Given a end node, return all the paths that reach it

    std::set<std::vector<uint32_t>> getPathsToNode(uint32_t nodeID) {

        std::set<std::vector<uint32_t>> paths;

        for (auto &e : execPaths()) {

            std::vector<uint32_t> path;

            auto f = std::find(e.begin(), e.end(), nodeID);

            if (f != e.end()) {

                for (auto p = e.begin(); p != f; ++p) {

                    uint32_t id = *p;

                    path.push_back(id);
                }

                path.push_back(*f);

                paths.insert(path);
            }
        }

        return paths;
    }

    std::multimap<uint32_t, uint32_t, std::greater<uint32_t>> sortNodes(uint32_t (LCOVfunction::*sortMethod)() const) {

        std::multimap<uint32_t, uint32_t, std::greater<uint32_t>> sorted_nodes;

        for (auto &it : this->nodes) {

            sorted_nodes.insert({it.data.get_UncovLines(), it.id});
        }

        return sorted_nodes;
    }

    void levelArrangement() {

        for (const auto &it : root_nodes) {

            Node &root = this->getNodeRef(it);

            root.setLevel(0);

            auto range = root.children();

            for (auto p = range.first; p != range.second; ++p) {

                Node &child = this->getNodeRef(p->second);

                child.setLevel(1);
            }
        }

        for (uint32_t current_level = 1; current_level < 2; current_level++) {

            for (const auto &it : level[current_level]) {

                Node &node = this->getNodeRef(it);

                auto range = node.children();

                for (auto p = range.first; p != range.second; ++p) {

                    Node &child = this->getNodeRef(p->second);

                    child.setLevel(current_level + 1);
                }
            }
        }
    }

    void backPropagation() {

        std::unordered_set<uint32_t> queue;

        for (const auto &it : leaf_nodes) {

            Node &leaf = this->getNodeRef(it);

            std::cout << "(Level 0) Leaf: " << leaf.data.getName() << std::endl;

            auto range = leaf.parents();

            for (auto p = range.first; p != range.second; ++p) {

                std::cout << "\t (Level 1) Parent: " << this->nodes[p->second].data.getName() << std::endl;

                Node &parent = this->getNodeRef(p->second);

                parent.propagatedValue += leaf.propagatedValue;

                queue.insert(parent.id);
            }
        }

        for (const auto &it : queue) {

            Node &node = this->getNodeRef(it);

            std::cout << "(Level 1) Node: " << node.data.getName() << std::endl;

            auto range = node.parents();

            for (auto p = range.first; p != range.second; ++p) {

                std::cout << "\t (Level 2) Parent: " << this->nodes[p->second].data.getName() << std::endl;

                Node &parent = this->getNodeRef(p->second);

                // parent.backPropagation();

                parent.propagatedValue += node.propagatedValue;

                // queue.insert(parent.id);
            }

            // queue.erase(it);
        }
    }

    void recursivePropagation() {}

    std::string printPath(const std::vector<uint32_t> &path, std::string format = "text") {

        std::stringstream ss;

        bool first = true;

        for (auto &id : path) {

            if (first) {
                if (format == "html")
                    ss << html::link(nodes[id].data.getName(), nodes[id].data.getPath());
                else
                    ss << nodes[id].data.getName() << " (" << nodes[id].data.getPath() << ")";
                first = false;
            } else {
                if (format == "html")
                    ss << " --> " << html::link(nodes[id].data.getName(), nodes[id].data.getPath());
                else
                    ss << " --> " << nodes[id].data.getName() << " (" << nodes[id].data.getPath() << ")";
            }
        }

        if (format == "text")
            ss << std::endl;

        return ss.str();
    }

    void printMermaid_coverage() {

        std::cout << "```mermaid" << std::endl;

        std::cout << "  graph TD;" << std::endl;

        for (const auto &it : children_nodes) {

            const Node &parent = this->getNodeRef(it.first);

            const Node &child = this->getNodeRef(it.second);

            std::cout << "  " << parent.data.getName() << "[\"" << parent.data.getName() << " (" << parent.data.get_UncovLines() << ") ("
                      << parent.propagatedValue << ")\"] --> " << child.data.getName() << "[\"" << child.data.getName() << " ("
                      << child.data.get_UncovLines() << ") (" << child.propagatedValue << ")\"];" << std::endl;

            // ares__readfile["ares__readfile (55)"];
        }

        std::cout << "```" << std::endl;
    }

    void printMermaid_level() {

        std::cout << "```mermaid" << std::endl;

        std::cout << "  graph TD;" << std::endl;

        for (int l = 0; l < level.size() - 1; l++) {

            for (const auto &it : level[l]) {

                const Node &parent = this->getNodeRef(it);

                auto range = parent.children();

                for (auto p = range.first; p != range.second; ++p) {

                    const Node &child = this->getNodeRef(p->second);

                    std::cout << "  " << parent.data.getName() << "[\"" << parent.data.getName() << " (" << parent.level << ")\"] --> "
                              << child.data.getName() << "[\"" << child.data.getName() << " (" << child.level << ")\"];" << std::endl;
                }

                // ares__readfile["ares__readfile (55)"];
            }
        }

        std::cout << "```" << std::endl;
    }

    LCOVfunction &getDataRef(uint32_t NodeID) { return nodes[NodeID].data; }

  private:
    class Node {
      public:
        Node() { this->graph = NULL; }

        Node(LCOVfunction data) {
            this->data = data;
            this->propagatedValue = data.get_UncovLines();

            this->graph = NULL;
        }

        bool isRoot() { return this->graph->root_nodes.count(this->id) > 0; }

        bool isLeaf() { return this->graph->leaf_nodes.count(this->id) > 0; }

        /*
        uint32_t score()
        {
            //return this->data.get_UncovLines();
        }
        */

        void setLevel(uint32_t level) {
            this->level = level;

            if (this->graph->level.size() <= level) {
                this->graph->level.resize(level + 1);
            }
            this->graph->level[level].insert(this->id);
        }

        /*
        Node(LCOVfunction data, CallGraph *graph)
        {
            this->data = data;
            this->graph = graph;
            this->id =
            this->propagatedValue = data.get_UncovLines();
        }
        */

        /*
        void insert(Node n){
            this->graph->child_nodes.insert({this->data, n.data});
        }
        */

        LCOVfunction data;

        std::pair<std::multimap<uint32_t, uint32_t>::const_iterator, std::multimap<uint32_t, uint32_t>::const_iterator> parents() const {

            // Check cache
            /*
            if(graph->parent_cache.count(this->data.getName()) > 0){
                return graph->parent_cache[this->data.getName()];
            }
            */

            auto range = graph->parent_nodes.equal_range(this->id);

            return range;
        }

        std::pair<std::multimap<uint32_t, uint32_t>::const_iterator, std::multimap<uint32_t, uint32_t>::const_iterator> children() const {

            // Check cache
            /*
            if(graph->parent_cache.count(this->data.getName()) > 0){
                return graph->parent_cache[this->data.getName()];
            }
            */

            auto range = graph->children_nodes.equal_range(this->id);

            return range;
        }

        // operator <
        bool operator<(const Node &other) const { return this->data < other.data; }

        // Operator ==
        bool operator==(const Node &other) const { return this->data == other.data; }

        // unordered_set needs this
        struct HashFunction {

            // operator () hash function integer
            std::size_t operator()(const Node &k) const { return std::hash<uint32_t>()(k.id); }

            /*
            std::size_t operator()(const Node *k) const
            {
                return std::hash<std::string>()(k->data.getName() + k->data.getPath() + std::to_string(k->data.get_UncovLines()));
            }
            */
        };

      private:
        CallGraph *graph; // Pointer to graph

        uint32_t id; // Node id in the graph. It has to be unique

        uint32_t level;

        uint32_t propagatedValue;

        friend class CallGraph;

        /*
        void backPropagation(){

            std::cout << "Backpropagating " << this->data.getName() << " at " << this->data.getPath() << std::endl;

            //Get parents
            std::vector<Node> parents = this->parents();

            //If no parents, return
            if(parents.size() == 0){
                return;
            }

            //Get the sum of all children
            int sum = 0;
            for(auto it = parents.begin(); it != parents.end(); ++it){
                sum += it->data.get_UncovLines();
            }

            //Get the average
            int avg = sum / parents.size();

            //Set the average to all parents
            for(auto it = parents.begin(); it != parents.end(); ++it){
                it->data.set_UncovLines(avg);
            }

            //Backpropagate
            for(auto it = parents.begin(); it != parents.end(); ++it){
                it->backPropagation();
            }
        }
        */
    };

    uint32_t insert(Node n) {

        n.id = this->nodes.size();
        n.graph = this;

        nodes.push_back(n);

        return n.id;
    }

    Node &getNodeRef(uint32_t id) { return nodes[id]; }

    // std::unordered_set<const Node, Node::HashFunction> nodes;

    std::vector<Node> nodes;

    std::multimap<uint32_t, uint32_t> parent_nodes;

    std::multimap<uint32_t, uint32_t> children_nodes;

    std::unordered_set<uint32_t> root_nodes;

    std::unordered_set<uint32_t> leaf_nodes;

    std::vector<std::unordered_set<uint32_t>> level;

    std::vector<std::vector<uint32_t>> prv__execPaths;
    bool execPathsComputed = false;

    std::unordered_map<std::string, std::vector<Node>> child_cache;

    std::unordered_map<std::string, std::vector<Node>> parent_cache;

    void prv__executionPaths() {

        // Depth first traversal

        std::vector<uint32_t> path;

        for (const auto &it : root_nodes) {

            Node &root = this->getNodeRef(it);

            auto range = root.children();

            path.push_back(it);

            prv__recursiveDFS(path);

            path.pop_back();
        }
    }

    void prv__recursiveDFS(std::vector<uint32_t> path) {

        uint32_t nodeID = path.back();

        Node &node = this->getNodeRef(nodeID);

        if (node.isLeaf()) {

            std::cout << "\nExecution Path: " << std::endl;

            bool first = true;

            for (const auto &it : path) {

                if (first) {
                    std::cout << nodes[it].data.getName();
                    first = false;
                } else {
                    std::cout << " --> " << nodes[it].data.getName();
                }
            }

            std::cout << std::endl;

            prv__execPaths.push_back(path);

            return;
        }

        auto range = node.children();

        for (auto p = range.first; p != range.second; ++p) {

            // If child node is previously in path we skip it (avoid cycles)
            if (std::find(path.begin(), path.end(), p->second) != path.end()) {
                continue;
            }

            path.push_back(p->second);

            prv__recursiveDFS(path);

            path.pop_back();
        }
    }
};

data::Table getEntryPoints(CallGraph &graph) {

    const uint32_t numFunctions = 10; // Numero de funciones a mostrar

    // html::table({"Company", "Contact", "Country"});

    data::Table table;

    // std::stringstream ss;

    // std::cout << std::endl << std::endl;

    // Vamos a sacar ahora los entrypoint para los nodos con mayor uncovered lines

    int n = 0;

    for (auto &it : graph.sortNodes(&LCOVfunction::get_UncovLines)) {

        uint32_t uncoveredLines = it.first;

        uint32_t nodeID = it.second;

        LCOVfunction &func = graph.getDataRef(nodeID);

        table << "Node: " << func.getName() << " : " << uncoveredLines << " uncovered lines";

        table.newRow();

        // Node &node = this->getNodeRef(it.second);

        // auto range = node.parents();

        // std::vector< std::vector<uint32_t> > execPaths;

        std::set<std::vector<uint32_t>> paths = graph.getPathsToNode(nodeID);

        bool first = true;

        for (auto &p : paths) {

            table << graph.printPath(p, "html");

            table.newRow();

            //

            // vector of LCOVfunction pointers
            std::vector<LCOVfunction *> dataPath;

            for (auto &n : p) {

                // Node &node = this->getNodeRef(n);

                // dataPath.push_back(&node.data);
            }

            // getPath();

            // std::cout << ss.str() << std::endl;
        }

        n++;
        if (n == numFunctions) {
            break;
        }
    }

    return table;

    // Get the node with the greatest number of uncovered lines
}

std::string noQuotes(std::string str) {

    if (str.size() < 2) {
        return str;
    }

    if (str[0] != '"' || str[str.size() - 1] != '"') {
        return str;
    }

    return str.substr(1, str.size() - 2);
}

data::Table githubAPI(std::string funcName) {

    data::Table table;

    std::string langFilter = "+language:C+language:C%2B%2B";

    std::string query = funcName + langFilter;

    nlohmann::json response = ghapi::search("code", query, 19);

    if (response.contains("Error")) {
        std::cerr << "Error: " << response["Error"] << std::endl;
        return table;
    }

    struct ResFile {

        std::string sha; // Unique

        std::string name;

        std::vector<std::string> fragment;
    };

    nlohmann::json j = response;

    int total_count;
    if (j.contains("total_count")) {
        total_count = j["total_count"];
    }

    if (j.contains("items")) {

        // nlohmann::json items = j["items"];

        // for (auto& el : object["response"]["items].items())

        // object["response"]

        std::unordered_map<std::string, ResFile> resultFiles; // sha -> file

        std::unordered_map<std::string, std::vector<std::string>> fragments; // fragment -> [sha, sha, sha, ...]

        std::unordered_multimap<std::string, std::string> repoPaths; // sha -> path

        for (auto &iter : j["items"].items()) {

            auto item = iter.value();

            ResFile file;

            file.sha = item["sha"];
            // HexToBytes(sha, result.sha);

            file.name = noQuotes(item["name"]);

            std::string html_url = noQuotes(item["html_url"]);
            std::string branch = "/tree/master/";
            std::string path = noQuotes(item["path"]);

            repoPaths.insert({file.sha, html_url + branch + path});

            if (resultFiles.count(file.sha)) {
                continue; // Skip innecesary computations
            }

            // table.setHeader({item["path"]});

            if (item.contains("text_matches")) {

                for (auto &it : item["text_matches"].items()) {

                    // std::cout << it << std::endl;

                    auto match = it.value();

                    std::string fragment = noQuotes(match["fragment"]);

                    if (fragments.count(fragment)) {
                        fragments[fragment].push_back(file.sha);
                    } else {
                        fragments.insert({fragment, {file.sha}});
                    }

                    // file.fragment.push_back(fragment);

                    // fragments.insert({fragment, file.sha});

                    // file.fragment.push_back(fragment);
                }
            }

            // std::string git_url = item["git_url"];

            // std::cout << git_url << std::endl;

            // auto value = item.value();

            // if(item.value().contains("git_url")){

            //}

            resultFiles.insert({file.sha, file});
        }

        // Join files by fragment

        // fragment -> sha

        for (auto &f : fragments) {

            std::set<std::string> uniqNames;

            uint32_t matches = 0;

            for (auto &sha : f.second) {

                uniqNames.insert(resultFiles[sha].name);

                matches += repoPaths.count(sha);
            }

            // Traverse uniqNames
            for (auto &name : uniqNames) {

                table << name << " ";
            }

            table.newColumn();

            table << matches << " matches";

            table.newRow();

            std::string fragment = f.first;

            data::CodeBlock cb(fragment);

            cb.highlight(funcName);

            table << cb;

            table.newRow();
            table.newRow();
            table.newRow();
        }
    }

    // std::cout << response_string << std::endl;

end:
    return table;
}

void foo(int total) {

    // progress::start();

    for (int i = 0; i < total; i++) {

        // std::cout << std::this_thread::get_id() << std::endl;

        // std::cout << "Hello World" << std::endl;

        sleep(1);

        progress::update(i, total, "");
    }

    // progress::end();
}

int main(int argc, char *argv[]) {

    {
        customOutput output(HTML);

        output.setOption("html", "style", "table {border-collapse: collapse;} table, th, td {border: 1px solid black;}");

        output.setFile("myTest2.html");

        data::Table table = githubAPI("ares_getnameinfo");

        output << table;

        output.dump();

        exit(0);
    }

    grThread th(myDump);

    // th.run(5);

    /*
    while(1==1){

        auto status = th.status();

        std::cout << "Progress = " << std::fixed << status.progress * 100.0 << std::scientific << "%" << std::endl;
        std::cout << "Status = " << status.currentItem << std::endl;
        std::cout << "Time Elapsed = " << format_duration(status.timeElapsed) << std::endl;
        std::cout << std::endl;

        sleep(2);
    }
    */

    auto start = std::chrono::high_resolution_clock::now();

    // dump();

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Time elapsed: " << duration.count() << " milliseconds" << std::endl;

    std::cout << "main id: " << std::this_thread::get_id() << std::endl;

    // grThread th(foo);

    // th.run(70);

    std::thread *th1;

    // th1 = new std::thread(foo, 90);

    // th1.join();

    /*
        progress::start();

        for(int i=0; i<60; i++){

            std::cout << "Hello World" << std::endl;

            sleep(1);

            progress::update(1, 25);

        }

        progress::end();
    */

    customOutput output(HTML);

    output.setOption("html", "style", "table {border-collapse: collapse;} table, th, td {border: 1px solid black;}");

    output.setFile("searchResults.html");

    data::Table table = githubAPI("ares_getnameinfo");

    output << table;

    output.dump();

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory>" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Error in " << __FILE__ << " : " << __LINE__ << " : " << __func__ << std::endl;

    std::vector<std::string> *files;

    files = gather_LCOV_files(argv[1]);

    std::vector<LCOVfunction> *functions = NULL;

    // Traverse input_file
    for (int i = 0; i < files->size(); i++) {

        std::vector<LCOVfunction> *tmp;

        LCOVfile lcov(files->at(i));

        if (!(tmp = lcov.scan_for_functions())) {
            std::cout << "Error: unable to retrieve functions from file " << files->at(i) << std::endl;
            exit(EXIT_FAILURE);
        }

        // Add tmp to functions
        if (!functions) {
            functions = tmp;
        } else {
            functions->insert(functions->end(), tmp->begin(), tmp->end());
        }
        /*
            std::cout << "Functions in file " << files->at(i) << std::endl;

            for(int i=0; i<functions->size(); i++){

                std::cout << functions->at(i).name() << std::endl;
            }

            std::cout << std::endl;
        */
    }

    std::sort(functions->begin(), functions->end(), std::greater{});

    // Traverse functions
    for (int i = 0; i < functions->size(); i++) {

        std::cout << functions->at(i).getName() << " at " << functions->at(i).getPath() << std::endl;
        std::cout << functions->at(i).get_UncovLines() << std::endl;
        std::cout << std::endl;
    }

    // Remove those functions that has 0 uncovered lines
    for (int i = 0; i < functions->size(); i++) {

        if (functions->at(i).get_UncovLines() == 0) {
            functions->resize(i);
            break;
        }
    }

    std::multimap<LCOVfunction, LCOVfunction> map;

    // Look for inclusive uncovered lines
    for (int i = 0; i < functions->size(); i++) {

        for (int j = 0; j < functions->size(); j++) {

            if (i == j) {
                continue;
            }

            if (functions->at(i).isCalledInside(functions->at(j))) {
                // functions->at(i).add_UncovLines(functions->at(j).get_UncovLines());

                map.insert({functions->at(i), functions->at(j)});

                std::cout << functions->at(i).getName() << " (" << functions->at(i).get_UncovLines() << ") "
                          << " --> " << functions->at(j).getName() << " (" << functions->at(j).get_UncovLines() << ")"
                          << ";" << std::endl;
            }
        }
    }

    CallGraph graph(map);

    graph.longestPath();

    auto paths = graph.sortPaths(&LCOVfunction::get_UncovLines);

    output.clear();

    data::Table tEntryPoints = getEntryPoints(graph);

    output << tEntryPoints;

    output.setFile("testing.html");

    output.dump();

    // output.print();

    // TODO: output << table;

    graph.levelArrangement();

    graph.printMermaid_level();

    graph.backPropagation();

    graph.printMermaid_coverage();

    /*
       //std::vector< std::vector<LCOVfunction> > callMatrix(functions->size(), std::vector<LCOVfunction>(0));
        std::map <LCOVfunction, std::vector<LCOVfunction> > map;


       for(int i=0; i<functions->size(); i++){

            //Insert an empty vector in map
            map.insert({functions->at(i), std::vector<LCOVfunction>()});

            for(int j=0; j<functions->size(); j++){

                if(i == j){
                    continue;
                }

                if(functions->at(i).isCalledInside(functions->at(j))){
                    //functions->at(i).add_UncovLines(functions->at(j).get_UncovLines());

                    //Insert into map
                    map[functions->at(i)].push_back(functions->at(j));

                    //callMatrix[i].push_back(functions->at(j));

                    //map.insert({functions->at(i).getName(), functions->at(j).getName()});

                    std::cout << functions->at(i).getName() << " (" << functions->at(i).get_UncovLines() << ") " << " --> " <<
       functions->at(j).getName() << " (" << functions->at(j).get_UncovLines() << ")" << ";" << std::endl;

                }
            }
        }
    */
    std::cout << std::endl << std::endl << std::endl;

    // Print map
    for (auto it = map.begin(); it != map.end(); ++it) {
        std::cout << it->first.getName() << " --> " << it->second.getName() << ";" << std::endl;
    }

    // Look for terminal nodes
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (map.find(it->second) == map.end()) {
            std::cout << it->second.getName() << " is a terminal node" << std::endl;
        }
    }

    std::cout << "size " << map.size() << std::endl;

    // Sort callMatrix by size
    // std::sort(callMatrix.begin(), callMatrix.end(), [](const std::vector<LCOVfunction>& a, const std::vector<LCOVfunction>& b) {return a.size() >
    // b.size();});

    // std::cout << "size " << callMatrix.size() << std::endl;

    // Traverse functions
    for (int i = 0; i < functions->size(); i++) {

        std::cout << functions->at(i).getName() << " at " << functions->at(i).getPath() << std::endl;
        std::cout << functions->at(i).get_UncovLines() << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

// Tree structure
