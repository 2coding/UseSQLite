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
#include "_USQLDatabase.hpp"

namespace usqlite {
    USQLConnection::USQLConnection(const std::string &fn) {
        _field = new _USQLDatabase(fn);
    }
    
    USQLConnection::~USQLConnection() {
        delete _field;
    }
    
    sqlite3 *USQLConnection::db() {
        return _USQL_DATABASE_CALL(db)();
    }
    
    bool USQLConnection::open() {
        return open(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    }
    
    bool USQLConnection::open(int flags) {
        _USQL_START_LOCK;
        return _USQL_DATABASE_CALL(open)(flags);
        _USQL_UNLOCK;
    }
    
    bool USQLConnection::isOpenning() const {
        _USQL_START_LOCK;
        return _USQL_DATABASE_CALL(isOpenning)();
        _USQL_UNLOCK;
    }
    
    bool USQLConnection::closeSync() {
        _USQL_START_LOCK;
        return _USQL_DATABASE_CALL(closeSync)();
        _USQL_UNLOCK;
    }
    
    void USQLConnection::close() {
        _USQL_START_LOCK;
        _USQL_DATABASE_CALL(close)();
        _USQL_UNLOCK;
    }
    
    void USQLConnection::setLastErrorCode(int code) {
        _USQL_START_LOCK;
        _USQL_DATABASE_CALL(setLastErrorCode)(code);
        _USQL_UNLOCK;
    }
    
    int USQLConnection::lastErrorCode() const {
        _USQL_START_LOCK;
        return _USQL_DATABASE_CALL(lastErrorCode)();
        _USQL_UNLOCK;
    }
    
    std::string USQLConnection::lastErrorMessage() {
        _USQL_START_LOCK;
        return _USQL_DATABASE_CALL(lastErrorMessage)();
        _USQL_UNLOCK;
    }
    
    bool USQLConnection::exec(const std::string &cmd) {
        if (cmd.empty()) {
            return false;
        }
        
        return exec(USQLCommand(cmd));
    }
    
    bool USQLConnection::exec(const usqlite::USQLCommand &cmd) {
        if (!isOpenning()) {
            return false;
        }
        
        _USQL_START_LOCK;
        _USQLStatement *stmt = _USQLStatement::create(cmd.command(), _USQL_CONNECTION_FIELD);
        bool ret = stmt->step();
        stmt->finilize();
        stmt->release();
        
        return ret;
        _USQL_UNLOCK;
    }
    
    USQLQuery USQLConnection::query(const std::string &cmd) {
        return query(USQLCommand(cmd));
    }
    
    USQLQuery USQLConnection::query(const usqlite::USQLCommand &cmd) {
        _USQL_START_LOCK;
        _USQLStatement *stmt = _USQLStatement::create(cmd.command(), _USQL_CONNECTION_FIELD);
        USQLQuery query(stmt);
        stmt->release();
        
        return query;
        _USQL_UNLOCK;
    }
    
    USQLStatement USQLConnection::compile(const std::string &cmd) {
        _USQL_START_LOCK;
        _USQLStatement *stmt = _USQLStatement::create(cmd, _USQL_CONNECTION_FIELD);
        USQLStatement statement(stmt);
        stmt->release();
        
        return statement;
        _USQL_UNLOCK;
    }
}