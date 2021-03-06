/**
 Copyright (c) 2015, 2coding
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include "StdCpp.hpp"
#include "Query.hpp"
#include "Statement.hpp"
#include "Connection.hpp"

namespace usql {
    Result Query::next() {
        return _stmt->query();
    }
    
    Result Query::reset() {
        return _stmt->reset();
    }
    
    int Query::columnCount() const {
        return _stmt->columnCount();
    }
    
    bool Query::availableIndex(int idx) const {
        return idx >= 0 && idx < columnCount();
    }
    
    int Query::columnIndexForName(const std::string &name) const {
        return _stmt->columnIndexForName(name);
    }
    
    ColumnType Query::typeForName(const std::string &name) const {
        int i = columnIndexForName(name);
        return typeForColumn(i);
    }
    
    ColumnType Query::typeForColumn(int i) const {
        if (!availableIndex(i)) {
            return _USQL_ENUM_VALUE(ColumnType, InvalidType);
        }
        
        return _stmt->typeForColumnIndex(i);
    }
    
    int Query::intForName(const std::string &name) {
        int i = columnIndexForName(name);
        return intForColumnIndex(i);
    }
    
    int Query::intForColumnIndex(int idx) {
        return _stmt->staticValueForColumnIndex<int>(idx, _USQL_ENUM_VALUE(ColumnType, Integer), sqlite3_column_int, USQL_ERROR_INTEGER);
    }
    
    bool Query::booleanForName(const std::string &name) {
        int i = columnIndexForName(name);
        return booleanForColumnIndex(i);
    }
    
    bool Query::booleanForColumnIndex(int idx) {
        return intForColumnIndex(idx);
    }
    
    int64_t Query::int64ForName(const std::string &name) {
        int i = columnIndexForName(name);
        return int64ForColumnIndex(i);
    }
    
    int64_t Query::int64ForColumnIndex(int idx) {
        return _stmt->staticValueForColumnIndex<sqlite3_int64>(idx, _USQL_ENUM_VALUE(ColumnType, Integer), sqlite3_column_int64, USQL_ERROR_INTEGER);
    }
    
    std::string Query::textForName(const std::string &name) {
        int i = columnIndexForName(name);
        return textForColumnIndex(i);
    }
    
    std::string Query::textForColumnIndex(int idx) {
        const unsigned char *txt = cstrForColumnIndex(idx);
        return std::string(txt ? (const char *)txt : USQL_ERROR_TEXT);
    }
    
    const unsigned char *Query::cstrForColumnIndex(int idx) {
        return _stmt->staticValueForColumnIndex<const unsigned char *>(idx, _USQL_ENUM_VALUE(ColumnType, Text), sqlite3_column_text, nullptr);
    }
    
    double Query::floatForName(const std::string &name) {
        int i = columnIndexForName(name);
        return floatForColumnIndex(i);
    }
    
    double Query::floatForColumnIndex(int idx) {
        return _stmt->staticValueForColumnIndex<double>(idx, _USQL_ENUM_VALUE(ColumnType, Float), sqlite3_column_double, USQL_ERROR_FLOAT);
    }
    
    std::time_t Query::datetimeForName(const std::string &name) {
        int i = columnIndexForName(name);
        return datetimeForColumnIndex(i);
    }
    
    std::time_t Query::datetimeForColumnIndex(int idx) {
        auto type = typeForColumn(idx);
        std::time_t ts = -1;
        if (type == _USQL_ENUM_VALUE(ColumnType, Integer)) {
            ts = intForColumnIndex(idx);
        }
        else if (type == _USQL_ENUM_VALUE(ColumnType, Text)) {
            std::string str = textForColumnIndex(idx);
            ts = Utils::str2tm(str);
        }
        else if (type == _USQL_ENUM_VALUE(ColumnType, Float)) {
            double day = floatForColumnIndex(idx);
            double t = (day - 2440587.5) * 86400.0;
            ts = static_cast<std::time_t>(t);
        }
        
        return ts;
    }
}