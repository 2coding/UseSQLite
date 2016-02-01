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

#include "_USQLStatement.hpp"
#include "_USQLDatabase.hpp"
#include "USQLDefs.hpp"

namespace usqlite {
    _USQLStatement::_USQLStatement(const std::string &cmd, _USQLDatabase *db)
    : _stmt(nullptr)
    , _command(cmd)
    , _db(db){
    }
    
    _USQLStatement::~_USQLStatement() {
        finilize();
    }
    
    _USQLStatement * _USQLStatement::create(const std::string &cmd, _USQLDatabase *db) {
        _USQLStatement *stmt = new _USQLStatement(cmd, db);
        return stmt;
    }
    
    void _USQLStatement::finilize() {
        if (!_stmt) {
            return;
        }
        
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
        _db->unregisterStatement(this);
        
        clearColumnInfo();
    }
    
    bool _USQLStatement::prepare() {
        if (_stmt) {
            return true;
        }
        
        sqlite3 *db = _db->db();
        int code = sqlite3_prepare_v2(db, _command.c_str(), static_cast<int>(_command.size() + 1), &_stmt, nullptr);
        if (!USQL_OK(code)) {
            _db->setLastErrorCode(code);
        }
        else {
            _db->registerStatement(this);
        }
        
        return USQL_OK(code);
    }
    
    bool _USQLStatement::reset() {
        clearColumnInfo();
        if (_stmt) {
            sqlite3_reset(_stmt);
            return true;
        }
        else {
            return prepare();
        }
    }
    
    bool _USQLStatement::step() {
        if (!reset()) {
            return false;
        }
        
        int code = sqlite3_step(_stmt);
        if (!USQL_STEP_OK(code)) {
            _db->setLastErrorCode(code);
        }
        
        return USQL_STEP_OK(code);
    }
    
    bool _USQLStatement::query() {
        if (!_stmt && !reset()) {
            return false;
        }
        
        int code = sqlite3_step(_stmt);
        if (!USQL_STEP_OK(code)) {
            _db->setLastErrorCode(code);
        }
        
        initColumnInfo();
        return USQL_QUERY_OK(code);
    }
    
    int _USQLStatement::columnIndexForName(const std::string &name) const {
        if (name.empty()) {
            return USQL_INVALID_COLUMN_INDEX;
        }
        
        auto iter = _columns.find(name);
        if (iter == _columns.end()) {
            return USQL_INVALID_COLUMN_INDEX;
        }
        
        return iter->second;
    }
    
    USQLColumnType _USQLStatement::typeForColumnIndex(int i) const {
        if (i < 0 || i >= _columnTypes.size()) {
            return USQLInvalidType;
        }
        
        return _columnTypes[i];
    }
    
    void _USQLStatement::initColumnInfo() {
        clearColumnInfo();
        if (!_stmt) {
            return ;
        }
        
        int count = sqlite3_column_count(_stmt);
        if (count <= 0) {
            return ;
        }
        
        for (int i = 0; i < count; ++i) {
            const char *name = sqlite3_column_name(_stmt, i);
            if (!name || name[0] == 0) {
                return ;
            }
            
            _columns[name] = i;
            
            USQLColumnType type = typeForColumn(i);
            if (type == USQLInvalidType) {
                return;
            }
            _columnTypes.push_back(type);
        }
    }
    
    USQLColumnType _USQLStatement::typeForColumn(int i) {
        USQLColumnType type = USQLInvalidType;
        int t = sqlite3_column_type(_stmt, i);
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
                
            case SQLITE_NULL:
                type = USQLNull;
                break;
        }
        
        return type;
    }
}
