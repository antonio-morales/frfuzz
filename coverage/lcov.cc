/* SPDX-License-Identifier: AGPL-3.0-only */
#include <fstream>
#include <iostream>

#include "coverage/lcov.h"
#include "utils/filesys.h"

namespace lcov {

Tracefile::Tracefile(const std::string &path) { path_ = path; }

bool Tracefile::parse() {

    std::string file_content = read_file(path_);
    if (file_content.empty()) {
        return false;
    }

    std::istringstream file_stream(file_content);

    std::string line;
    while (std::getline(file_stream, line)) {
        if (!parse_line(line)) {
            std::cerr << "Error parsing line: " << line << std::endl;
            return false;
        }
    }

    /*
      std::ifstream in(path_);
      if (!in) return false;
      std::string line;
      while (std::getline(in, line)) lines_.push_back(line);

    */
    return true;
}

bool Tracefile::parse_line(const std::string &line) {

    if (line.empty()) {
        return false;
    }

    // End of Record
    if (line == "end_of_record") {

        // End of the current record
        currentFunction = nullptr;
        currentSourceFile = nullptr;

        return true;
    }

    auto colon = line.find(':');
    if (colon == std::string_view::npos) {
        std::cerr << "Error: Invalid line format (no colon found)" << std::endl;
        return false;
    }

    std::string key = line.substr(0, colon);
    std::string val = line.substr(colon + 1);

    // Test Name
    if (key == "TN") {

        currentSourceFile = nullptr;
        currentFunction = nullptr;

        // Source File
    } else if (key == "SF") {

        for (const auto &sf : sourceFiles_) {
            if (sf->getPath() == val) {
                std::cerr << "Error: Duplicate source file found: " << val << std::endl;
                return false;
            }
        }

        SourceFile *sf = new SourceFile(val, this);
        this->sourceFiles_.push_back(sf);
        currentSourceFile = sf;
        currentFunction = nullptr; // Reset current function when a new source file is encountered

        // Function Name
    } else if (key == "FN") {

        if (currentSourceFile == nullptr) {
            std::cerr << "Error: FN record found before SF record" << std::endl;
            return false;
        }

        // FN contains 3 fields separated by comma: line number start, line number end, function name
        int start_line = 0;
        int end_line = 0;
        std::string func_name;

        size_t first_comma = val.find(',');
        if (first_comma == std::string::npos) {
            std::cerr << "Error: Invalid FN record format" << std::endl;
            return false;
        }
        size_t second_comma = val.find(',', first_comma + 1);
        if (second_comma == std::string::npos) {
            std::cerr << "Error: Invalid FN record format" << std::endl;
            return false;
        }
        try {
            start_line = std::stoi(val.substr(0, first_comma));
            end_line = std::stoi(val.substr(first_comma + 1, second_comma - first_comma - 1));
            func_name = val.substr(second_comma + 1);
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid FN record format" << std::endl;
            return false;
        }

        Function *func = new Function(func_name, start_line, end_line, currentSourceFile);
        currentSourceFile->addFunction(func);
        currentFunction = func;

        // Function Data
    } else if (key == "FNDA") {

        currentFunction = nullptr;

        // FNDA contains 2 fields separated by comma: execution count, function name
        int exec_count = 0;
        std::string func_name;
        size_t comma = val.find(',');
        if (comma == std::string::npos) {
            std::cerr << "Error: Invalid FNDA record format" << std::endl;
            return false;
        }
        try {
            exec_count = std::stoi(val.substr(0, comma));
            func_name = val.substr(comma + 1);
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid FNDA record format" << std::endl;
            return false;
        }

        // Find the function in the current source file
        if (currentSourceFile == nullptr) {
            std::cerr << "Error: FNDA record found before SF record" << std::endl;
            return false;
        }

        bool found = false;
        for (const auto &func : currentSourceFile->functions()) {
            if (func->getName() == func_name) {
                func->setExecutionCount(exec_count);
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "Error: FNDA record refers to unknown function: " << func_name << std::endl;
            return false;
        }

        // Functions Found
    } else if (key == "FNF") {

        int num_functions = 0;

        try {
            num_functions = std::stoi(val);
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid FNF record format" << std::endl;
            return false;
        }

        currentSourceFile->setNumFunctions(num_functions);

        // Functions Hit
    } else if (key == "FNH") {

        int functions_hit = 0;

        try {
            functions_hit = std::stoi(val);
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid FNH record format" << std::endl;
            return false;
        }

        currentSourceFile->setFunctionsHit(functions_hit);

        // Line Data
    } else if (key == "DA") {

        currentFunction = nullptr;

        // DA contains 2 fields separated by comma: line number, execution count
        int line_number = 0;
        int exec_count = 0;
        size_t comma = val.find(',');
        if (comma == std::string::npos) {
            std::cerr << "Error: Invalid DA record format" << std::endl;
            return false;
        }
        try {
            line_number = std::stoi(val.substr(0, comma));
            exec_count = std::stoi(val.substr(comma + 1));
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid DA record format" << std::endl;
            return false;
        }

        currentSourceFile->setLineHits(line_number, exec_count);

        // Lines Found
    } else if (key == "LF") {

        int lines_found = 0;

        try {
            lines_found = std::stoi(val);
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid LF record format" << std::endl;
            return false;
        }

        if (currentSourceFile == nullptr) {
            std::cerr << "Error: LF record found before SF record" << std::endl;
            return false;
        }

        // Check if lines_found matches the number of lcov lines in the source file
        if (lines_found != currentSourceFile->getLcovLines()) {
            std::cerr << "Warning: Number of lines found (" << lines_found << ") does not match total lines in source file ("
                      << currentSourceFile->getLcovLines() << ") in file: " << currentSourceFile->getPath() << std::endl;
        }

        // Lines Hit
    } else if (key == "LH") {

        int lines_hit = 0;

        try {
            lines_hit = std::stoi(val);
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error: Invalid LH record format" << std::endl;
            return false;
        }

        if (currentSourceFile == nullptr) {
            std::cerr << "Error: LH record found before SF record" << std::endl;
            return false;
        }

        // Check if lines_hit matches the number of lines with hits in the source file
        if (lines_hit != currentSourceFile->getHittedLines()) {
            std::cerr << "Warning: Number of lines hit (" << lines_hit << ") does not match number of lines with hits in source file ("
                      << currentSourceFile->getHittedLines() << ") in file: " << currentSourceFile->getPath() << std::endl;
        }

    } else {
        std::cerr << "Warning: Unknown record type: " << key << std::endl;
        return false;
    }

    /*
    Summary of Common LCOV Tags

        TN:	Test Name
        SF:	Source File path
        FN:	Function name & line
        FNDA:	Function execution count
        DA:	Line execution count
        LF:	Lines Found
        LH:	Lines Hit
        FNF:	Functions Found
        FNH:	Functions Hit
        BRDA:	Branch data (for conditionals)
        BRF: / BRH:	Branches Found / Hit
        end_of_record	End of file’s data
    */

    return true;
}

std::ostream &operator<<(std::ostream &os, const Tracefile &tf) {

    for (const auto &sf : tf.sourceFiles_) {
        os << "\nSource File: " << sf->getPath() << "\n";
        // We also want to print lcov_lines_ and hitted_lines_
        os << "  Total Lines: " << sf->getLcovLines() << ", Hitted Lines: " << sf->getHittedLines() << "\n";
        // We also want to print functions number and functions hit
        os << "  Total Functions: " << sf->getNumFunctions() << ", Functions Hit: " << sf->getFunctionsHit() << "\n";
        for (const auto &f : sf->functions()) {
            os << "\n  Function: " << f->getName() << " (Execution Count: " << f->getExecutionCount() << ")\n";
            os << "    " << f->getLinesHit() << "/" << f->getLcovLines() << " lines covered" << "\n";
        }
    }
    return os;
}

SourceFile::SourceFile(std::string path, Tracefile *parent) {

    parentTracefile = parent;

    path_ = path;

    file_content_ = read_file(path_);
    if (file_content_.empty()) {
        std::cerr << "Error reading file: " << path_ << std::endl;
        return;
    }

    total_lines_ = std::count(file_content_.begin(), file_content_.end(), '\n');
    // TODO: Print it only if debug mode is enabled
    // std::cout << "Total lines in " << path_ << ": " << total_lines_ << std::endl;

    lines_.reserve(total_lines_);

    // Copy these lines into lines_ vector
    std::istringstream file_stream(file_content_);
    std::string line;
    int line_number = 1;
    while (std::getline(file_stream, line)) {
        Line l(line_number, 0, line, nullptr);
        lines_.push_back(l);
        line_number++;
    }
}

void SourceFile::setNumFunctions(int n) {
    num_functions_ = n;

    if (num_functions_ != functions_.size()) {
        std::cerr << "Warning: Number of functions parsed (" << functions_.size() << ") does not match number of functions reported ("
                  << num_functions_ << ") in file: " << path_ << std::endl;
    }
}

void SourceFile::setFunctionsHit(int n) {
    functions_hit_ = n;

    int count = 0;
    for (const auto &func : functions_) {
        if (func->getExecutionCount() > 0) {
            count++;
        }
    }
    if (functions_hit_ != count) {
        std::cerr << "Warning: Number of functions hit (" << count << ") does not match number of functions with execution count > 0 ("
                  << functions_hit_ << ") in file: " << path_ << std::endl;
    }
}

void SourceFile::setLineHits(int line, int hits) {

    if (hits > 0) {
        lines_[line - 1].setHits(hits);
        hitted_lines_++;
    }

    lines_[line - 1].setLcovTrackedLine(true);
}

int SourceFile::getLcovLines() const {

    int count = 0;
    for (const auto &line : lines_) {
        if (line.isLcovTrackedLine()) {
            count++;
        }
    }
    return count;
}

Function::Function(std::string fn, int sl, int el, SourceFile *sf) {

    funcName_ = fn;

    if (sl > el) {
        std::cerr << "Error: start_line cannot be greater than end_line for function " << fn << " in file " << sf->getPath() << std::endl;
        exit(EXIT_FAILURE);
    }
    start_line_ = sl;
    end_line_ = el;
    sourceFile = sf;

    // Now we need to point these lines to this function
    for (int i = start_line_; i <= end_line_; i++) {

        Line *line = &sourceFile->lines()[i - 1];

        line->setParentFunction(this);

        lines_.push_back(line);
    }
}

std::string Function::getSourceText() const {
    std::string text = "";
    for (const auto &line : lines_) {
        text += line->getSourceText() + "\n";
    }
    return text;
}

std::string Function::getCovText() const {
    std::string text = "";
    for (const auto &line : lines_) {
        text += line->getCovText() + "\n";
    }
    return text;
}

int Function::getLcovLines() const {

    int count = 0;
    for (const auto &line : lines_) {
        if (line->isLcovTrackedLine()) {
            count++;
        }
    }
    return count;
}

int Function::getLinesHit() const {
    int hit = 0;
    for (const auto &l : lines_) {
        if (l->getHits() > 0) {
            hit++;
        }
    }
    return hit;
}

} // namespace lcov