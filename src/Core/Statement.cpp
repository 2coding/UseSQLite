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

#include "Statement.hpp"
#include "Utils.hpp"
#include "Connection.hpp"

namespace usql {
    Statement::Statement(const std::string &cmd, _WeakDatabase db)
    : _stmt(nullptr)
    , _command(cmd)
    , _db(db)
    , _parametersCount(0) {
    }
    
    Statement::~Statement() {
        finilize();
    }
    
    void Statement::finilize() {
        if (!_stmt) {
            return;
        }
        
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
        if (!_db.expired()) {
            _db.lock()->unregisterStatement(this);
        }
        
        clearColumnInfo();
        clearParameters();
    }
    
    Result Statement::prepare() {
        if (_stmt) {
            return Result::success();
        }
        
        clearParameters();
        
        if (_db.expired()) {
            return Result::error();
        }
        
        auto ptr = _db.lock();
        sqlite3 *db = ptr->db();
        Result ret(sqlite3_prepare_v2(db, _command.c_str(), static_cast<int>(_command.size() + 1), &_stmt, nullptr), _db);
        if (ret) {
            ptr->registerStatement(this);
            initParameters();
        }
        
        return ret;
    }
    
    Result Statement::reset() {
        clearColumnInfo();
        if (_stmt) {
            return Result(sqlite3_reset(_stmt), _db);
        }
        else {
            return prepare();
        }
    }
    
    Result Statement::step() {
        Result ret = reset();
        if (!ret) {
            return ret;
        }
        
        return Result::step(sqlite3_step(_stmt), _db);
    }
    
    Result Statement::query() {
        if (!_stmt) {
            return Result::error();
        }
        
        Result ret = Result::query(sqlite3_step(_stmt), _db);
        if (ret) {
            initColumnInfo();
        }
        
        return ret;
    }
    
    int Statement::columnIndexForName(const std::string &name) const {
        if (name.empty()) {
            return USQL_INVALID_COLUMN_INDEX;
        }
        
        auto iter = _columns.find(name);
        if (iter == _columns.end()) {
            return USQL_INVALID_COLUMN_INDEX;
        }
        
        return iter->second;
    }
    
    ColumnType Statement::typeForColumnIndex(size_t i) const {
        if (i >= _columnTypes.size()) {
            return _USQL_ENUM_VALUE(ColumnType, InvalidType);
        }
        
        return _columnTypes[i];
    }
    
    void Statement::initColumnInfo() {
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
            
            ColumnType type = typeForColumn(i);
            if (type == _USQL_ENUM_VALUE(ColumnType, InvalidType)) {
                return;
            }
            _columnTypes.push_back(type);
        }
    }
    
    ColumnType Statement::typeForColumn(int i) {
        ColumnType type = _USQL_ENUM_VALUE(ColumnType, InvalidType);
        int t = sqlite3_column_type(_stmt, i);
        switch (t) {
            case SQLITE_INTEGER:
                type = _USQL_ENUM_VALUE(ColumnType, Integer);
                break;
                
            case SQLITE_TEXT:
                type = _USQL_ENUM_VALUE(ColumnType, Text);
                break;
                
            case SQLITE_FLOAT:
                type = _USQL_ENUM_VALUE(ColumnType, Float);
                break;
                
            case SQLITE_BLOB:
                type = _USQL_ENUM_VALUE(ColumnType, Blob);
                break;
                
            case SQLITE_NULL:
                type = _USQL_ENUM_VALUE(ColumnType, Null);
                break;
        }
        
        return type;
    }
    
    void Statement::initParameters() {
        if (!_stmt) {
            return;
        }
        
        clearParameters();
        
        _parametersCount = sqlite3_bind_parameter_count(_stmt);
        if (_parametersCount <= 0) {
            return;
        }
        
        for (int i = 1; i <= _parametersCount; ++i) {
            const char *name = sqlite3_bind_parameter_name(_stmt, i);
            if (name && name[0]) {
                _nameParameters[name] = i;
            }
        }
    }
    
    int Statement::parameterIndexForName(const std::string &name) const {
        if (name.empty()) {
            return USQL_INVALID_PARAMETER_INDEX;
        }
        
        auto iter = _nameParameters.find(name);
        return iter == _nameParameters.end() ? USQL_INVALID_PARAMETER_INDEX : iter->second;
    }
}
