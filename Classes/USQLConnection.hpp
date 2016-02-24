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

#include "USQLStdCpp.hpp"
#include "USQLQuery.hpp"
#include "USQLCommand.hpp"
#include "USQLStatement.hpp"

namespace usqlite {
    class _USQLStatement;
    
    class USQLConnection : public USQLNoCopyable
    {
    public:
        USQLConnection(const std::string &filename);
        virtual ~USQLConnection();
        
        bool open();
        bool open(int flags);
        bool isOpenning() const {
            return _db != nullptr;
        }
        
        bool close();
        
        inline void setLastErrorCode(int code) {
            _errorCode = code;
        }
        inline int lastErrorCode() const {
            return _errorCode;
        }
        
        inline std::string lastErrorMessage() {
            return _db ? sqlite3_errmsg(_db) : sqlite3_errstr(lastErrorCode());
        }
        
        inline sqlite3 *db() {
            return _db;
        }
        
    public:
        bool exec(const std::string &cmd);
        bool transaction(USQLTransactionType type, std::tr1::function<bool()> action);
        
        bool tableExists(const std::string &tablename);
        
    protected:
        friend class _USQLStatement;
        
        void registerStatement(_USQLStatement *stmt);
        void unregisterStatement(_USQLStatement *stmt);
        void finilizeAllStatements(bool finilized);
        
    private:
        std::string _filename;
        sqlite3 *_db;
        
        int _errorCode;
        
        std::list<_USQLStatement *> _statements;
    };
}

#endif /* USQLConnection_hpp */
