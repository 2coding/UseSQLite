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
#include "Query.hpp"
#include "Cursor.hpp"

namespace usql {
    DBConnection::DBConnection(const std::string &fn)
    : _filename(fn.empty() ? ":memory" : fn)
    , _db(Database::create()){
    }
    
    DBConnection::~DBConnection() {
        close();
    }
    
    Result DBConnection::open() {
        return open(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    }
    
    Result DBConnection::open(int flags) {
        return Result(_db->open(_filename, flags), _db);
    }
    
    Result DBConnection::close() {
        Result ret(_db->close(), _db);
        if (ret) {
            _functions.clear();
        }
        
        return ret;
    }
    
    Result DBConnection::exec(const std::string &cmd) {
        if (cmd.empty()) {
            return false;
        }
        
        if (!isOpenning()) {
            return false;
        }
        
        Cursor cursor(cmd, *this);
        return cursor.exec();
    }
    
    Result DBConnection::beginTransaction(TransactionType type) {
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
    
    Result DBConnection::commit() {
        return this->exec("COMMIT");
    }
    
    Result DBConnection::rollback() {
        return this->exec("ROLLBACK");
    }
    
    Result DBConnection::transaction(TransactionType type, tr1::function<bool()> action) {
        Result ret = beginTransaction(type);
        if (!ret) {
            return ret;
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
    
    Result DBConnection::attachDatabase(const std::string &filename, const std::string &schema) {
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
    
    Result DBConnection::registerFunction(Function *func) {
        int opt = 0;
        int code = SQLITE_OK;
        
        typedef void(*sqlite_func)(sqlite3_context*,int,sqlite3_value**);
        sqlite_func xfunc = nullptr, xstep = nullptr;
        void (*xfinal)(sqlite3_context *) = nullptr;
        
        if (!func || !isOpenning()) {
            goto failed;
        }
        
        if (func->name.empty()) {
            goto failed;
        }
        
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
        
        code = sqlite3_create_function_v2(_db->db(), func->name.c_str(), func->argumentCount(), opt, func, xfunc, xstep, xfinal, Function::xDestroy);
        if (!_USQL_OK(code)) {
            delete func;
            return Result(code, _db);
        }
        
        _functions.push_back(func->name);
        return Result(code, _db);
        
    failed:
        delete func;
        return false;
    }
    
    void DBConnection::unregisterFunction(const std::string name) {
        if (!isOpenning()) {
            return;
        }
        
        sqlite3_create_function(_db->db(), name.c_str(), -1, 0, nullptr, nullptr, nullptr, nullptr);
        _functions.remove(name);
    }
    
    void DBConnection::unregisterAllFunctions() {
        for (auto iter = _functions.begin(); iter != _functions.end(); ++iter) {
            sqlite3_create_function(_db->db(), iter->c_str(), -1, 0, nullptr, nullptr, nullptr, nullptr);
        }
        _functions.clear();
    }
}