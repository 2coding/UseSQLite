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
#include "USQLSatement.hpp"

namespace usqlite {
    USQLQuery::USQLQuery(USQLSatement *stmt)
    : _stmt(stmt) {
        _stmt->retain();
    }
    
    USQLQuery::USQLQuery(const USQLQuery &other) {
        _stmt = other._stmt;
        _stmt->retain();
    }
    
    USQLQuery::~USQLQuery() {
        _stmt->release();
    }
    
    bool USQLQuery::next() {
        return _stmt->query();
    }
    
    bool USQLQuery::reset() {
        _columns.clear();
        return _stmt->reset();
    }
    
    int USQLQuery::columnCount() {
        return sqlite3_column_count(_stmt->statement());
    }
    
    bool USQLQuery::availableIndex(int idx) {
        return idx >= 0 && idx < columnCount();
    }
    
    bool USQLQuery::initNameColumn() {
        if (_columns.size() > 0) {
            return true;
        }
        
        sqlite3_stmt *stmt = _stmt->statement();
        if (!stmt) {
            return false;
        }
        
        int count = columnCount();
        if (count == 0) {
            return false;
        }
        
        for (int i = 0; i < count; ++i) {
            const char *name = sqlite3_column_name(stmt, i);
            if (!name || name[0] == 0) {
                return false;
            }
            
            _columns[name] = i;
        }
        
        return true;
    }
    
    int USQLQuery::columnForName(const std::string &name) {
        if (name.empty() || !initNameColumn()) {
            return USQL_INVALID_COLUMN_INDEX;
        }
        
        auto iter = _columns.find(name);
        if (iter == _columns.end()) {
            return USQL_INVALID_COLUMN_INDEX;
        }
        
        int idx = iter->second;
        return availableIndex(idx) ? idx : USQL_INVALID_COLUMN_INDEX;
    }
    
    USQLColumnType USQLQuery::typeForColumn(int i) {
        if (!availableIndex(i)) {
            return USQLInvalidType;
        }
        
        USQLColumnType type = USQLInvalidType;
        int t = sqlite3_column_type(_stmt->statement(), i);
        switch (t) {
            case SQLITE_INTEGER:
                type = USQLInteger;
                break;
            
            case SQLITE_TEXT:
                type = USQLText;
                break;
                
            case SQLITE_FLOAT:
                type = USQLFloat;
                break;
                
            case SQLITE_BLOB:
                type = USQLBlob;
                break;
        }
        
        return type;
    }
    
    USQLColumnType USQLQuery::typeForName(const std::string &name) {
        int i = columnForName(name);
        return typeForColumn(i);
    }
    
    int USQLQuery::hasName(const std::string &name, USQLColumnType type) {
        int idx = columnForName(name);
        return typeForColumn(idx) == type ? idx : USQL_INVALID_COLUMN_INDEX;
    }
    
    int USQLQuery::intForName(const std::string &name) {
        int i = 0;
        if ((i = hasName(name, USQLInteger)) < 0) {
            return USQL_ERROR_INTEGER;
        }
        
        return sqlite3_column_int(_stmt->statement(), i);
    }
    
    std::string USQLQuery::textForName(const std::string &name) {
        int i = 0;
        if ((i = hasName(name, USQLText)) < 0) {
            return USQL_ERROR_TEXT;
        }
        
        const unsigned char *txt = sqlite3_column_text(_stmt->statement(), i);
        return std::string(txt ? (const char *)txt : USQL_ERROR_TEXT);
    }
    
    double USQLQuery::doubleForName(const std::string &name) {
        int i = 0;
        if ((i = hasName(name, USQLFloat)) < 0) {
            return USQL_ERROR_FLOAT;
        }
        
        return sqlite3_column_double(_stmt->statement(), i);
    }
    
//    std::string USQLQuery::blobForName(const std::string &name) {
//        int i = 0;
//        if ((i = hasName(name, SQLITE_BLOB)) < 0) {
//            return USQL_ERROR_BLOB;
//        }
//        
//        const void *data = sqlite3_column_blob(_stmt->statement(), i);
//        if (!data) {
//            return USQL_ERROR_BLOB;
//        }
//        
//        int size = sqlite3_column_bytes(_stmt->statement(), i);
//        if (size <= 0) {
//            return USQL_ERROR_BLOB;
//        }
//        
//        return std::string((const char *)data, size);
//    }
}