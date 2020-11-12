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

#include "Cursor.hpp"
#include "Utils.hpp"
#include "Statement.hpp"
#include "Connection.hpp"

namespace usql {
    static inline sqlite3_destructor_type bindValueDestructorType(BindType type) {
        sqlite3_destructor_type t = SQLITE_TRANSIENT;
        switch (type) {
            case _USQL_ENUM_VALUE(BindType, Copy):
                t = SQLITE_TRANSIENT;
                break;
                
            case _USQL_ENUM_VALUE(BindType, Static):
                t = SQLITE_STATIC;
                break;
                
            default:
                break;
        }
        
        return t;
    }
    
    Cursor::Cursor(const std::string &cmd, Connection &db)
    : _stmt(nullptr) {
        _stmt = new Statement(cmd, db.database());
        _stmt->reset();
    }
    
    Cursor::~Cursor() {
        close();
        delete _stmt;
    }
    
    void Cursor::close() {
        _stmt->finilize();
    }
    
    Result Cursor::bind(const std::string &key, int value) {
        return _stmt->bindName(key, BindValue(value));
    }
    
    Result Cursor::bind(const std::string &key, sqlite3_int64 value) {
        return _stmt->bindName(key, BindValue(value));
    }
    
    Result Cursor::bind(const std::string &key, double value) {
        return _stmt->bindName(key, BindValue(value));
    }
    
    Result Cursor::bind(const std::string &key, const std::string &value, BindType opt) {
        return _stmt->bindName(key, BindValue(value.c_str(), -1, bindValueDestructorType(opt)));
    }
    
    Result Cursor::bind(const std::string &key, const void *blob, int count, BindType opt) {
        if (!blob || count <= 0) {
            return false;
        }
        
        return _stmt->bindName(key, BindValue(blob, count, bindValueDestructorType(opt)));
    }
    
    Result Cursor::bind(int index, int value) {
        return _stmt->bindIndex(index, BindValue(value));
    }
    
    Result Cursor::bind(int index, sqlite3_int64 value) {
        return _stmt->bindIndex(index, BindValue(value));
    }
    
    Result Cursor::bind(int index, double value) {
        return _stmt->bindIndex(index, BindValue(value));
    }
    
    Result Cursor::bind(int index, const std::string &value, BindType opt) {
        return _stmt->bindIndex(index, BindValue(value.c_str(), -1, bindValueDestructorType(opt)));
    }
    
    Result Cursor::bind(int index, const void *blob, int count, BindType opt) {
        if (!blob || count <= 0) {
            return false;
        }
        
        return _stmt->bindIndex(index, BindValue(blob, count, bindValueDestructorType(opt)));
    }
    
    Result Cursor::exec() {
        return _stmt->step();
    }
}