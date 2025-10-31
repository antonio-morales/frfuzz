/* SPDX-License-Identifier: AGPL-3.0-only */
#include "error.h"

void criticalError(const std::source_location location) {

    std::string str = "Error in '";

    str += location.function_name();
    str += "' at ";
    str += basename(const_cast<char *>(location.file_name()));
    str += ":";
    str += std::to_string(location.line());
    // str += basename(location.file_name); //In GCC 12 we can use __FILE_NAME__ instead

    std::perror(str.c_str());

    std::exit(EXIT_FAILURE);
}

void criticalError(std::string msg, const std::source_location location) {

    std::string str = "Error in '";

    str += location.function_name();
    str += "' at ";
    str += basename(const_cast<char *>(location.file_name()));
    str += ":";
    str += std::to_string(location.line());
    // str += basename(location.file_name); //In GCC 12 we can use __FILE_NAME__ instead

    std::cerr << str << " " << msg << std::endl;

    std::exit(EXIT_FAILURE);
}