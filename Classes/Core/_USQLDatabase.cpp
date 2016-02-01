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

#include "_USQLDatabase.hpp"
#include "USQLDefs.hpp"
#include "_USQLStatement.hpp"

namespace usqlite {
    _USQLDatabase::_USQLDatabase(const std::string &fn)
    : _filename(fn.empty() ? ":memory" : fn)
    , _errorCode(SQLITE_OK)
    , _db(nullptr) {
        
    }
    
    _USQLDatabase::~_USQLDatabase() {
        close();
    }
    
    bool _USQLDatabase::open(int flags) {
        if (_filename.empty()) {
            return false;
        }
        
        if (_db) {
            return true;
        }
        
        _errorCode = sqlite3_open_v2(_filename.c_str(), &_db, flags, nullptr);
        return USQL_OK(_errorCode);
    }
    
    bool _USQLDatabase::closeSync() {
        if (!_db) {
            return true;
        }
        
        int code = sqlite3_close(_db);
        if (code == SQLITE_BUSY) {
            //busy
            finilizeAllStatements(true);
            
            code = sqlite3_close(_db);
        }
        
        if (USQL_OK(code)) {
            _db = nullptr;
        }
        
        _errorCode = code;
        return USQL_OK(_errorCode);
    }
    
    void _USQLDatabase::close() {
        if (!_db) {
            return;
        }
        
        finilizeAllStatements(false);
        
        _errorCode = sqlite3_close_v2(_db);
        _db = nullptr;
    }
    
    void _USQLDatabase::registerStatement(_USQLStatement *stmt) {
        if (!stmt) {
            return;
        }
        
        USQL_START_LOCK;
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            return;
        }
        
        _statements.push_back(stmt);
        stmt->retain();
        USQL_UNLOCK;
    }
    
    void _USQLDatabase::unregisterStatement(_USQLStatement *stmt) {
        if (!stmt) {
            return;
        }
        
        USQL_START_LOCK;
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            _statements.remove(stmt);
            stmt->release();
        }
        USQL_UNLOCK;
    }
    
    void _USQLDatabase::finilizeAllStatements(bool finilized) {
        USQL_START_LOCK;
        if (_statements.size() == 0) {
            return;
        }
        
        auto list = _statements;
        _statements.clear();
        
        for (auto iter = list.begin(); iter != list.end(); ++iter) {
            _USQLStatement *stmt = *iter;
            if (finilized) {
                stmt->finilize();
            }
            stmt->release();
        }
        USQL_UNLOCK;
    }
}