/* SPDX-License-Identifier: AGPL-3.0-only */
#include "output.h"

bool customOutput::dump() {

    try {
        // std::ifstream in;
        // in.exceptions(std::ifstream::failbit);
        // in.open(path);

        outFile.exceptions(std::ofstream::failbit);

        outFile.open(currentPath, std::ios::out | std::ios::trunc);

    } catch (const std::ifstream::failure &fail) {
        criticalError();
    }

    if (!outFile.is_open()) {
        return false;
    }

    if (format == TEXT) {

        for (auto data : buffer) {
            outFile << data;
        }

    } else if (format == HTML) {

        html::Document doc;

        if (options.html.contains("style")) {
            doc.setStyle(options.html["style"]);
        }

        for (auto data : buffer) {
            doc << data;
        }

        outFile << doc;
    }

    outFile.close();

    /*
            for (auto data : buffer)
            {
                if(format == TEXT){

                    outFile << data->toText();

                }else if(format == HTML){

                    html::document doc;
                }
            }
    */
    /*
    if(format == TEXT){

        outFile << buffer.str();

    }else if(format == HTML){

        html::document doc;


        doc.setBody(buffer.str());


        outFile << doc;
        //outFile << html::header();
    }
    */

    // fflush(outFile);

    return true;
}
