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

#ifndef USQLConnection_hpp
#define USQLConnection_hpp

#include <string>
#include <sqlite3.h>
#include <list>
#include "USQLNoCopyable.hpp"

namespace usqlite {
    class USQLSatement;
    
    class USQLConnection : public USQLNoCopyable
    {
    public:
        USQLConnection(const std::string &filename);
        ~USQLConnection();
        
        bool open();
        bool open(int flags);
        
        void close();
        bool closeSync();
        
        void setLastErrorCode(int code) {
            _errorCode = code;
        }
        
        int lastErrorCode() const {
            return _errorCode;
        }
        
        std::string lastErrorMessage() {
            return _db ? sqlite3_errmsg(_db) : sqlite3_errstr(lastErrorCode());
        }
        
        sqlite3 *db() {
            return _db;
        }
        
        void registerStatement(USQLSatement *stmt);
        void unregisterStatement(USQLSatement *stmt);
        
    private:
        void finilizeAllStatements(bool finilized);
        
    public:
        const std::string filename;
        
    private:
        sqlite3 *_db;
        int _errorCode;
        
        std::list<USQLSatement *> _statements;
    };
}

#endif /* USQLConnection_hpp */
