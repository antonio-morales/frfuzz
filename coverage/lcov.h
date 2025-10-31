/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once
#include <string>
#include <vector>

#include "utils/filesys.h"

namespace lcov {

class Line;
class Function;
class SourceFile;

class Tracefile {

  public:
    explicit Tracefile(const std::string &path);

    ~Tracefile() {
        for (auto sf : sourceFiles_) {
            delete sf;
        }
    }

    bool parse();

    const std::vector<SourceFile *> &sourceFiles() const { return sourceFiles_; }

    const std::vector<Function *> &allFunctions() const { return allFunctions_; }

    void addFunction(Function *f) { allFunctions_.push_back(f); }

  private:
    std::string path_;
    // std::vector<std::string> lines_;

    // All source files in the tracefile
    std::vector<SourceFile *> sourceFiles_;

    // All functions in the tracefile
    std::vector<Function *> allFunctions_;

    // Current source file being parsed
    SourceFile *currentSourceFile = nullptr;

    // Current function being parsed
    Function *currentFunction = nullptr;

    bool parse_line(const std::string &line);

    // Overload << operator
    friend std::ostream &operator<<(std::ostream &os, const Tracefile &tf);
};

class SourceFile {

  public:
    SourceFile(std::string path, Tracefile *parent);

    ~SourceFile() {
        for (auto f : functions_) {
            delete f;
        }
    }

    void addFunction(Function *f) {
        functions_.push_back(f);
        parentTracefile->addFunction(f);
    }

    std::string getPath() const { return path_; }

    const std::vector<Function *> &functions() const { return functions_; }

    void setNumFunctions(int n);

    void setFunctionsHit(int n);

    void setLineHits(int line, int hits);

    int getLcovLines() const;

    int getHittedLines() const { return hitted_lines_; }

    int getNumFunctions() const { return num_functions_; }

    int getFunctionsHit() const { return functions_hit_; }

    // Return a reference to lines_
    std::vector<Line> &lines() { return lines_; }

  private:
    Tracefile *parentTracefile = nullptr;

    std::string path_;
    std::string file_content_;

    int total_lines_ = 0; // Total lines in the source file
    // int lcov_lines_ = 0; // Lines tracked by LCOV
    int hitted_lines_ = 0; // Lines with hits

    int num_functions_ = 0;
    int functions_hit_ = 0;

    std::vector<Function *> functions_;
    std::vector<Line> lines_;
};

class Function {

  private:
    SourceFile *sourceFile;

    std::string funcName_;

    int start_line_;
    int end_line_;

    int execution_count_ = 0;

    std::vector<std::string> parameters;

    std::vector<Line *> lines_;

    // int num_lines_ = 0;
    // int lines_hit_ = 0;

    // text_pos body_start;

    // int uncoveredLines;

  public:
    // Function(std::string n, std::string p, text_pos body) {
    Function(std::string fn, int sl, int el, SourceFile *sf);

    ~Function() {}

    /*
    void push(Line l) {
        lines.push_back(l);
        if (l.count == 0) {
            uncoveredLines++;
        }
    }
    */

    Line *getLine(int numLine) const {

        // We need to check that numLine is within the function range
        if (numLine < start_line_ || numLine > end_line_) {
            std::cerr << "Error: Line number " << numLine << " is out of range for function " << funcName_ << " (" << start_line_ << "-" << end_line_
                      << ")" << std::endl;
            return nullptr;
        }

        return lines_[numLine - start_line_];
    }

    std::string getName() const { return funcName_; }

    std::string getSourceText() const;

    std::string getCovText() const;

    int getStartLine() const { return start_line_; }

    int getEndLine() const { return end_line_; }

    void setExecutionCount(int count) { execution_count_ = count; }

    int getExecutionCount() const { return execution_count_; }

    int getNumLines() const { return lines_.size(); }

    int getLcovLines() const;

    int getLinesHit() const;

    int getUncovLines() const { return getLcovLines() - getLinesHit(); }

    /*
    std::string getPath() const {
        return path;
    }
    */

    SourceFile *getSourceFile() const { return sourceFile; }

    int size() const { return lines_.size(); }

    /*
    bool isCalledInside(Function func) {

        int i = body_start.line;

        if (lines[i].source.find(func.getName(), body_start.pos) != std::string::npos) {
            return true;
        }

        i++;

        for (; i < lines.size(); i++) {
            if (lines[i].source.find(func.getName()) != std::string::npos) {
                return true;
            }
        }

        return false;
    }
    */

    // overload << operator
    /*
    friend std::ostream &operator<<(std::ostream &os, const Function &f) {
        os << "\n"
           << f.funcName_ << " :\n"
           << std::endl;
        for (auto l : f.lines) {
            os << l << std::endl;
        }
        return os;
    }
    */

    /*
    bool operator<(const Function &f) const {
        return (this->uncoveredLines < f.uncoveredLines);
    }

    bool operator>(const Function &f) const {
        return (this->uncoveredLines > f.uncoveredLines);
    }
    */

    /*
    bool operator==(const Function &f) const {
        return (this->funcName == f.funcName && this->path == f.path && this->size() == f.size() && uncoveredLines == f.uncoveredLines);
    }
    */

    /*
    // unordered_set needs this
    struct HashFunction {

        std::size_t operator()(const Function &k) const {
            return std::hash<std::string>()(k.funcName + k.path + std::to_string(k.size()) + std::to_string(k.uncoveredLines));
        }
    };
    */
};

class Line {

  private:
    int lineNumber = 0;
    bool lcov_tracked_line = false;
    int hits = 0;
    std::string source = "";

    Function *function = nullptr;

  public:
    Line() {}

    Line(int l, int h, std::string s, Function *f) {

        lineNumber = l;
        hits = h;
        source = s;
        function = f;
    }

    ~Line() {}

    bool isLcovTrackedLine() const { return lcov_tracked_line; }

    void setLcovTrackedLine(bool tracked) { lcov_tracked_line = tracked; }

    void setParentFunction(Function *f) { function = f; }

    void setHits(int h) { hits = h; }

    int getHits() const { return hits; }

    std::string getSourceText() const { return source; }

    std::string getCovText() const {
        std::string covText = "";
        covText += std::to_string(lineNumber) + "            " + std::to_string(hits) + " :     " + source;
        return covText;
    }

    /*
    std::string str() {
        std::string str = std::to_string(line) + " : " + std::to_string(count) + " : " + source;
        return str;
    }
    */

    // Overload <<
    friend std::ostream &operator<<(std::ostream &os, const Line &l) {
        os << l.lineNumber << " | " << l.hits << " | " << l.source;
        return os;
    }
};

} // namespace lcov
