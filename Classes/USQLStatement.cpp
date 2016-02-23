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

#include "USQLStatement.hpp"
#include "_USQLUtils.hpp"
#include "_USQLStatement.hpp"

namespace usqlite {
    USQLStatement::USQLStatement(const std::string &cmd, USQLConnection &con)
    : USQLQuery(cmd, con) {}
    
    bool USQLStatement::bind(const std::string &key, int value) {
        return _stmt->bindName<int>(key, sqlite3_bind_int, value);
    }
    
    bool USQLStatement::bind(const std::string &key, int64_t value) {
        return _stmt->bindName<sqlite3_int64>(key, sqlite3_bind_int64, value);
    }
    
    bool USQLStatement::bind(const std::string &key, double value) {
        return _stmt->bindName<double>(key, sqlite3_bind_double, value);
    }
    
    bool USQLStatement::bind(const std::string &key, const std::string &value) {
        return _stmt->bindName(key, sqlite3_bind_text, value.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    bool USQLStatement::bind(const std::string &key, const void *blob, int count) {
        if (!blob || count <= 0) {
            return false;
        }
        
        return _stmt->bindName(key, sqlite3_bind_blob, blob, count, SQLITE_TRANSIENT);
    }
    
    bool USQLStatement::bind(int index, int value) {
        return _stmt->bindIndex<int>(index, sqlite3_bind_int, value);
    }
    
    bool USQLStatement::bind(int index, int64_t value) {
        return _stmt->bindIndex<sqlite3_int64>(index, sqlite3_bind_int64, value);
    }
    
    bool USQLStatement::bind(int index, double value) {
        return _stmt->bindIndex<double>(index, sqlite3_bind_double, value);
    }
    
    bool USQLStatement::bind(int index, const std::string &value) {
        return _stmt->bindIndex(index, sqlite3_bind_text, value.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    bool USQLStatement::bind(int index, const void *blob, int count) {
        if (!blob || count <= 0) {
            return false;
        }
        
        return _stmt->bindIndex(index, sqlite3_bind_blob, blob, count, SQLITE_TRANSIENT);
    }
    
    bool USQLStatement::exec() {
        return _stmt->step();
    }
}