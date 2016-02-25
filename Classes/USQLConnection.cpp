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

#include "USQLConnection.hpp"
#include "_USQLStatement.hpp"

namespace usqlite {
    USQLConnection::USQLConnection(const std::string &fn)
    : _filename(fn.empty() ? ":memory" : fn)
    , _errorCode(SQLITE_OK)
    , _db(nullptr) {
        
    }
    
    USQLConnection::~USQLConnection() {
        close();
    }
    
    bool USQLConnection::open() {
        return open(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    }
    
    bool USQLConnection::open(int flags) {
        if (_filename.empty()) {
            return false;
        }
        
        if (_db) {
            return true;
        }
        
        _errorCode = sqlite3_open_v2(_filename.c_str(), &_db, flags, nullptr);
        return _USQL_OK(_errorCode);
    }
    
    bool USQLConnection::close() {
        if (!_db) {
            return true;
        }
        
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
    
    void USQLConnection::registerStatement(_USQLStatement *stmt) {
        if (!stmt) {
            return;
        }
        
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            return;
        }
        
        _statements.push_back(stmt);
    }
    
    void USQLConnection::unregisterStatement(_USQLStatement *stmt) {
        if (!stmt) {
            return;
        }
        
        auto ret = std::find(_statements.begin(), _statements.end(), stmt);
        if (ret != _statements.end()) {
            _statements.remove(stmt);
        }
    }
    
    void USQLConnection::finilizeAllStatements(bool finilized) {
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
        }
    }
    
    bool USQLConnection::exec(const std::string &cmd) {
        if (cmd.empty()) {
            return false;
        }
        
        if (!isOpenning()) {
            return false;
        }
        
        _USQLStatement stmt(cmd, this);
        return stmt.step();
    }
    
    bool USQLConnection::transaction(USQLTransactionType type, std::tr1::function<bool()> action) {
        if (!isOpenning()) {
            return false;
        }
        
        std::stringstream ss;
        ss<<"BEGIN ";
        if (type == USQLTransactionType::USQLDeferred) {
            ss<<"DEFERRED";
        }
        else if(type == USQLTransactionType::USQLImmediate) {
            ss<<"IMMEDIATE";
        }
        else {
            ss<<"EXCLUSIVE";
        }
        ss<<" TRANSACTION";
        
        if (!this->exec(ss.str())) {
            return false;
        }
        
        if (action()) {
            return this->exec("COMMIT");
        }
        else {
            return this->exec("ROLLBACK");
        }
    }
    
    bool USQLConnection::tableExists(const std::string &tablename, const std::string &schema) {
        if (tablename.empty() || !isOpenning()) {
            return false;
        }
        
        std::stringstream buf;
        buf<<"SELECT count(*) FROM ";
        if (!schema.empty()) {
            buf<<schema<<".";
        }
        buf<<"sqlite_master WHERE type='table' AND name='"<<tablename<<"'";
        USQLQuery query(buf.str(), *this);
        if (!query.next()) {
            return false;
        }
        
        return query.intForColumnIndex(0) > 0;
    }
    
    std::vector<std::string> USQLConnection::allTables(const std::string &schema) {
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
        
        USQLQuery query(buf.str(), *this);
        while (query.next()) {
            tables.push_back(query.textForName("name"));
        }
        
        return tables;
    }
    
    USQLConnection::TableInfo USQLConnection::tableInfo(const std::string &name, const std::string &schema) {
        USQLConnection::TableInfo table;
        if (name.empty()) {
            return table;
        }
        
        std::stringstream buf;
        buf<<"PRAGMA ";
        if (!schema.empty()) {
            buf<<schema<<".";
        }
        buf<<"table_info("<<name<<")";
        
        USQLQuery query(buf.str(), *this);
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
    
    bool USQLConnection::attachDatabase(const std::string &filename, const std::string &schema) {
        if (filename.empty() || schema.empty()) {
            return false;
        }
        
        std::stringstream buf;
        buf<<"ATTACH DATABASE '"<<filename<<"' AS "<<schema;
        return exec(buf.str());
    }
    
    void USQLConnection::detachDatabase(const std::string &schema) {
        if (schema.empty()) {
            return ;
        }
        
        std::stringstream buf;
        buf<<"DETACH DATABASE "<<schema;
        exec(buf.str());
    }
    
    std::vector<USQLConnection::DatabaseInfo> USQLConnection::allDatabase() {
        std::vector<USQLConnection::DatabaseInfo> dbs;
        
        USQLQuery query("PRAGMA database_list", *this);
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
}