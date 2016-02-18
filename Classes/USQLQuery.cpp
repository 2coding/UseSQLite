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

#include "USQLStdCpp.hpp"
#include "USQLQuery.hpp"
#include "_USQLStatement.hpp"

namespace usqlite {
    USQLQuery::USQLQuery(USQLObject *stmt)
    : _field(stmt) {}
    
    USQLQuery::~USQLQuery() {
        close();
        delete _field;
    }
    
    void USQLQuery::close() {
        _USQL_STATEMENT_CALL(finilize)();
    }
    
    bool USQLQuery::next() {
        return _USQL_STATEMENT_CALL(query)();
    }
    
    bool USQLQuery::reset() {
        return _USQL_STATEMENT_CALL(reset)();
    }
    
    int USQLQuery::columnCount() const {
        return _USQL_STATEMENT_CALL(columnCount)();
    }
    
    bool USQLQuery::availableIndex(int idx) const {
        return idx >= 0 && idx < columnCount();
    }
    
    int USQLQuery::columnIndexForName(const std::string &name) const {
        return _USQL_STATEMENT_CALL(columnIndexForName)(name);
    }
    
    USQLColumnType USQLQuery::typeForName(const std::string &name) const {
        int i = columnIndexForName(name);
        return typeForColumn(i);
    }
    
    USQLColumnType USQLQuery::typeForColumn(int i) const {
        if (!availableIndex(i)) {
            return USQLColumnType::USQLInvalidType;
        }
        
        return _USQL_STATEMENT_CALL(typeForColumnIndex)(i);
    }
    
    int USQLQuery::intForName(const std::string &name) {
        int i = columnIndexForName(name);
        return intForColumnIndex(i);
    }
    
    int USQLQuery::intForColumnIndex(int idx) {
        return _USQL_STATEMENT_CALL(staticValueForColumnIndex<int>)(idx, USQLColumnType::USQLInteger, sqlite3_column_int, USQL_ERROR_INTEGER);
    }
    
    int64_t USQLQuery::int64ForName(const std::string &name) {
        int i = columnIndexForName(name);
        return int64ForColumnIndex(i);
    }
    
    int64_t USQLQuery::int64ForColumnIndex(int idx) {
        return _USQL_STATEMENT_CALL(staticValueForColumnIndex<sqlite3_int64>)(idx, USQLColumnType::USQLInteger, sqlite3_column_int64, USQL_ERROR_INTEGER);
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
        return _USQL_STATEMENT_CALL(staticValueForColumnIndex<const unsigned char *>)(idx, USQLColumnType::USQLText, sqlite3_column_text, nullptr);
    }
    
    double USQLQuery::floatForName(const std::string &name) {
        int i = columnIndexForName(name);
        return floatForColumnIndex(i);
    }
    
    double USQLQuery::floatForColumnIndex(int idx) {
        return _USQL_STATEMENT_CALL(staticValueForColumnIndex<double>)(idx, USQLColumnType::USQLFloat, sqlite3_column_double, USQL_ERROR_FLOAT);
    }
}