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
    
    bool USQLQuery::initNameColumn() {
        if (_columns.size() > 0) {
            return true;
        }
        
        sqlite3_stmt *stmt = _stmt->statement();
        if (!stmt) {
            return false;
        }
        
        int count = sqlite3_column_count(stmt);
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
    
    int USQLQuery::hasName(const std::string &name) {
        if (name.empty() || !initNameColumn()) {
            return -1;
        }
        
        auto iter = _columns.find(name);
        if (iter == _columns.end()) {
            return -1;
        }
        
        return iter->second;
    }
    
    int USQLQuery::intForName(const std::string &name) {
        int i = 0;
        if ((i = hasName(name)) < 0) {
            return USQL_ERROR_INTEGER;
        }
        
        return sqlite3_column_int(_stmt->statement(), i);
    }
}