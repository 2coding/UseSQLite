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

#include "DBConnection.hpp"
#include "Statement.hpp"

namespace usql {
    DBConnection::DBConnection(const std::string &fn)
    : _filename(fn.empty() ? ":memory" : fn)
    , _errorCode(SQLITE_OK)
    , _db(nullptr) {
        
    }
    
    DBConnection::~DBConnection() {
        close();
    }
    
    bool DBConnection::open() {
        return open(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    }
    
    bool DBConnection::open(int flags) {
        if (_filename.empty()) {
            return false;
        }
        
        if (_db) {
            return true;
        }
        
        _errorCode = sqlite3_open_v2(_filename.c_str(), &_db, flags, nullptr);
        return _USQL_OK(_errorCode);
    }
    
    bool DBConnection::close() {
        if (!_db) {
            return true;
        }
        
        unregisterAllFunctions();
        
        int code = sqlite3_close(_db);
        if (code == SQLITE_BUSY) {
            //busy
            finilizeAllStatements(true);
            code = sqlite3_close(_db);
        }
        
        if (_USQL_OK(code)) {
            _db = nullptr;
        }
        
        _errorCode = code;
        return _USQL_OK(_errorCode);
    }
    
    void DBConnection::registerStatement(Statement *stmt) {
        if (!stmt) {
            return;
        }
        
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            return;
        }
        
        _statements.push_back(stmt);
    }
    
    void DBConnection::unregisterStatement(Statement *stmt) {
        if (!stmt) {
            return;
        }
        
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            _statements.remove(stmt);
        }
    }
    
    void DBConnection::finilizeAllStatements(bool finilized) {
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
    
    bool DBConnection::exec(const std::string &cmd) {
        if (cmd.empty()) {
            return false;
        }
        
        if (!isOpenning()) {
            return false;
        }
        
        Statement stmt(cmd, this);
        return stmt.step();
    }
    
    bool DBConnection::beginTransaction(TransactionType type) {
        if (!isOpenning()) {
            return false;
        }
        
        std::stringstream ss;
        ss<<"BEGIN ";
        if (type == TransactionType::Deferred) {
            ss<<"DEFERRED";
        }
        else if(type == TransactionType::Immediate) {
            ss<<"IMMEDIATE";
        }
        else {
            ss<<"EXCLUSIVE";
        }
        ss<<" TRANSACTION";
        
        return this->exec(ss.str());
    }
    
    bool DBConnection::commit() {
        return this->exec("COMMIT");
    }
    
    bool DBConnection::rollback() {
        return this->exec("ROLLBACK");
    }
    
    bool DBConnection::transaction(TransactionType type, tr1::function<bool()> action) {        
        if (!beginTransaction(type)) {
            return false;
        }
        
        if (action()) {
            return commit();
        }
        else {
            return rollback();
        }
    }
    
    bool DBConnection::tableExists(const std::string &tablename, const std::string &schema) {
        if (tablename.empty() || !isOpenning()) {
            return false;
        }
        
        std::stringstream buf;
        buf<<"SELECT count(*) FROM ";
        if (!schema.empty()) {
            buf<<schema<<".";
        }
        buf<<"sqlite_master WHERE type='table' AND name='"<<tablename<<"'";
        Query query(buf.str(), *this);
        if (!query.next()) {
            return false;
        }
        
        return query.intForColumnIndex(0) > 0;
    }
    
    std::vector<std::string> DBConnection::allTables(const std::string &schema) {
        std::vector<std::string> tables;
        if (!isOpenning()) {
            return tables;
        }
        
        std::stringstream buf;
        buf<<"SELECT name FROM ";
        if (!schema.empty()) {
            buf<<schema<<".";
        }
        buf<<"sqlite_master WHERE type='table'";
        
        Query query(buf.str(), *this);
        while (query.next()) {
            tables.push_back(query.textForName("name"));
        }
        
        return tables;
    }
    
    DBConnection::TableInfo DBConnection::tableInfo(const std::string &name, const std::string &schema) {
        DBConnection::TableInfo table;
        if (name.empty()) {
            return table;
        }
        
        std::stringstream buf;
        buf<<"PRAGMA ";
        if (!schema.empty()) {
            buf<<schema<<".";
        }
        buf<<"table_info("<<name<<")";
        
        Query query(buf.str(), *this);
        while (query.next()) {
            ColumnInfo column;
            column.name = query.textForName("name");
            column.type = query.textForName("type");
            column.nullable = !query.intForName("notnull");
            column.defaultValue = query.textForName("dflt_value");
            
            int idx = query.intForName("pk");
            if (idx) {
                table.primaryKeys.push_back(column.name);
            }
            
            table.columndefs.push_back(column);
        }
        
        table.name = name;
        return table;
    }
    
    bool DBConnection::attachDatabase(const std::string &filename, const std::string &schema) {
        if (filename.empty() || schema.empty()) {
            return false;
        }
        
        std::stringstream buf;
        buf<<"ATTACH DATABASE '"<<filename<<"' AS "<<schema;
        return exec(buf.str());
    }
    
    void DBConnection::detachDatabase(const std::string &schema) {
        if (schema.empty()) {
            return ;
        }
        
        std::stringstream buf;
        buf<<"DETACH DATABASE "<<schema;
        exec(buf.str());
    }
    
    std::vector<DBConnection::DatabaseInfo> DBConnection::allDatabase() {
        std::vector<DBConnection::DatabaseInfo> dbs;
        
        Query query("PRAGMA database_list", *this);
        while (query.next()) {
            if (query.columnCount() < 3) {
                continue;
            }
            
            std::string name = query.textForColumnIndex(1);
            std::string file = query.textForColumnIndex(2);
            dbs.push_back(DatabaseInfo(name, file));
        }
        query.close();
        
        return dbs;
    }
    
    bool DBConnection::registerFunction(Function *func) {
        int argc = -1;
        int opt = 0;
        
        typedef void(*sqlite_func)(sqlite3_context*,int,sqlite3_value**);
        sqlite_func xfunc = nullptr, xstep = nullptr;
        void (*xfinal)(sqlite3_context *) = nullptr;
        
        if (!func || !isOpenning()) {
            goto failed;
        }
        
        if (func->name.empty()) {
            goto failed;
        }
        
        argc = std::max(-1, func->argumentCount());
        
        opt |= static_cast<int>(func->encoding);
        if (func->deterministic) {
            opt |= SQLITE_DETERMINISTIC;
        }
        
        if (dynamic_cast<AggregateFunction *>(func)) {
            xstep = Function::xFunc;
            xfinal = AggregateFunction::xFinal;
        }
        else {
            xfunc = Function::xFunc;
        }
        
        _errorCode = sqlite3_create_function_v2(db(), func->name.c_str(), argc, opt, func, xfunc, xstep, xfinal, Function::xDestroy);
        if (!_USQL_OK(_errorCode)) {
            goto failed;
        }
        
        _functions.push_back(func->name);
        return true;
        
    failed:
        delete func;
        return false;
    }
    
    void DBConnection::unregisterFunction(const std::string name) {
        if (!isOpenning()) {
            return;
        }
        
        sqlite3_create_function(db(), name.c_str(), -1, 0, nullptr, nullptr, nullptr, nullptr);
        _functions.remove(name);
    }
    
    void DBConnection::unregisterAllFunctions() {
        for (auto iter = _functions.begin(); iter != _functions.end(); ++iter) {
            sqlite3_create_function(db(), iter->c_str(), -1, 0, nullptr, nullptr, nullptr, nullptr);
        }
        _functions.clear();
    }
}