/* SPDX-License-Identifier: AGPL-3.0-only */
#include "data.h"

namespace data {

Table::Table() {
    rows.resize(numRows);
    rows[0].resize(1);
}

Table::~Table() {}

void Table::newColumn() {
    currCol++;
    rows[numRows - 1].resize(currCol + 1);
}

void Table::newRow() {
    numRows++;
    rows.resize(numRows);
    rows[numRows - 1].resize(1);
    currCol = 0;
}

const std::vector<std::string> &Table::header() const { return prvHeader; }

const std::vector<std::vector<std::vector<Object *>>> &Table::data() const { return rows; }

/*
std::string Table::str()
{
}
*/

std::ostream &operator<<(std::ostream &stream, Object &d) {

    stream << d;

    return stream;
}

} // namespace data
