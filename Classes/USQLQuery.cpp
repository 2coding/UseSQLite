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

#include "USQLQuery.hpp"
#include "_USQLStatement.hpp"

#define _USQL_QUERY_FIELD (dynamic_cast<_USQLStatement *>(_field))

namespace usqlite {
    USQLQuery::USQLQuery(USQLObject *stmt)
    : _field(dynamic_cast<_USQLStatement *>(stmt)) {
        _USQL_QUERY_FIELD->retain();
    }
    
    USQLQuery::USQLQuery(const USQLQuery &other) {
        _field = other._field;
        _USQL_QUERY_FIELD->retain();
    }
    
    USQLQuery::~USQLQuery() {
        _USQL_QUERY_FIELD->release();
    }
    
    bool USQLQuery::next() {
        return _USQL_QUERY_FIELD->query();
    }
    
    bool USQLQuery::reset() {
        return _USQL_QUERY_FIELD->reset();
    }
    
    int USQLQuery::columnCount() const {
        return _USQL_QUERY_FIELD->columnCount();
    }
    
    bool USQLQuery::availableIndex(int idx) const {
        return idx >= 0 && idx < columnCount();
    }
    
    int USQLQuery::columnIndexForName(const std::string &name) const {
        return _USQL_QUERY_FIELD->columnIndexForName(name);
    }
    
    USQLColumnType USQLQuery::typeForName(const std::string &name) const {
        int i = columnIndexForName(name);
        return typeForColumn(i);
    }
    
    USQLColumnType USQLQuery::typeForColumn(int i) const {
        if (!availableIndex(i)) {
            return USQLInvalidType;
        }
        
        return _USQL_QUERY_FIELD->typeForColumnIndex(i);
    }
    
    int USQLQuery::intForName(const std::string &name) {
        int i = columnIndexForName(name);
        return intForColumnIndex(i);
    }
    
    int USQLQuery::intForColumnIndex(int idx) {
        return _USQL_QUERY_FIELD->staticValueForColumnIndex(idx, USQLInteger, sqlite3_column_int, USQL_ERROR_INTEGER);
    }
    
    int64_t USQLQuery::int64ForName(const std::string &name) {
        int i = columnIndexForName(name);
        return int64ForColumnIndex(i);
    }
    
    int64_t USQLQuery::int64ForColumnIndex(int idx) {
        return _USQL_QUERY_FIELD->staticValueForColumnIndex<sqlite3_int64>(idx, USQLInteger, sqlite3_column_int64, USQL_ERROR_INTEGER);
    }
    
    std::string USQLQuery::textForName(const std::string &name) {
        int i = columnIndexForName(name);
        return textForColumnIndex(i);
    }
    
    std::string USQLQuery::textForColumnIndex(int idx) {
        const unsigned char *txt = cstrForColumnIndex(idx);
        return std::string(txt ? (const char *)txt : USQL_ERROR_TEXT);
    }
    
    const unsigned char *USQLQuery::cstrForColumnIndex(int idx) {
        return _USQL_QUERY_FIELD->staticValueForColumnIndex<const unsigned char *>(idx, USQLText, sqlite3_column_text, nullptr);
    }
    
    double USQLQuery::floatForName(const std::string &name) {
        int i = columnIndexForName(name);
        return floatForColumnIndex(i);
    }
    
    double USQLQuery::floatForColumnIndex(int idx) {
        return _USQL_QUERY_FIELD->staticValueForColumnIndex(idx, USQLFloat, sqlite3_column_double, USQL_ERROR_FLOAT);
    }
}