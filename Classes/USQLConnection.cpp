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
#include "USQLDefs.hpp"

namespace usqlite {
    USQLConnection::USQLConnection(const std::string &fn)
    : filename(fn.empty() ? ":memory" : fn)
    , _errorCode(SQLITE_OK) {
        
    }
    
    USQLConnection::~USQLConnection() {
        close();
    }
    
    bool USQLConnection::open() {
        return open(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    }
    
    bool USQLConnection::open(int flags) {
        if (_db) {
            return true;
        }
        
        _errorCode = sqlite3_open_v2(filename.c_str(), &_db, flags, nullptr);
        return USQL_OK(_errorCode);
    }
    
    bool USQLConnection::closeSync() {
        if (!_db) {
            return true;
        }
        
        int code = sqlite3_close(_db);
        if (code == SQLITE_BUSY) {
            //busy
        }
        
        _errorCode = code;
        return USQL_OK(_errorCode);
    }
    
    void USQLConnection::close() {
        if (_db) {
            sqlite3_close_v2(_db);
            _db = nullptr;
        }
    }
}