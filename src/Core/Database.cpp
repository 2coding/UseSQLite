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

#include "Database.hpp"
#include "Utils.hpp"
#include "Statement.hpp"

namespace usql {
    Database::~Database() {
        close();
    }
    
    int Database::open(const std::string &filepath, int flags) {
        if (filepath.empty()) {
            return SQLITE_ERROR;
        }
        
        if (isOpening()) {
            return SQLITE_OK;
        }
        
        return sqlite3_open_v2(filepath.c_str(), &_db, flags, nullptr);
    }
    
    int Database::close() {
        if (!isOpening()) {
            return SQLITE_OK;
        }
        
        int code = sqlite3_close(_db);
        if (code == SQLITE_BUSY) {
            finilizeAllStatements(true);
            code = sqlite3_close(_db);
        }
        
        if (_USQL_OK(code)) {
            _db = nullptr;
        }
        
        return code;
    }
    
    std::string Database::errorDescription(int code) const {
        if (!isOpening()) {
            return _USQL_SQLITE_ERRSTR(code);
        }
        
        return sqlite3_errmsg(_db);
    }
    
    void Database::registerStatement(Statement *stmt) {
        if (!stmt) {
            return;
        }
        
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            return;
        }
        
        _statements.push_back(stmt);
    }
    
    void Database::unregisterStatement(Statement *stmt) {
        if (!stmt) {
            return;
        }
        
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            _statements.remove(stmt);
        }
    }
    
    void Database::finilizeAllStatements(bool finilized) {
        if (_statements.size() == 0) {
            return;
        }
        
        auto list = _statements;
        _statements.clear();
        
        for (auto iter = list.begin(); iter != list.end(); ++iter) {
            Statement *stmt = *iter;
            if (finilized) {
                stmt->finilize();
            }
        }
    }
}