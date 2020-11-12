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

#ifndef Database_hpp
#define Database_hpp

#include "StdCpp.hpp"
#include "Object.hpp"

namespace usql {
    class Database;
    typedef tr1::shared_ptr<Database> _Database;
    typedef tr1::weak_ptr<Database> _WeakDatabase;
    
    class Statement;
    class Database : public NoCopyable
    {
    public:
        static _Database create() {
            return tr1::shared_ptr<Database>(new Database());
        }
        
        ~Database();
        
        int open(const std::string &filepath, int flags);
        
        int close();
        
        bool isOpening() const {
            return _db != nullptr;
        }
        
        sqlite3 *db() {
            return _db;
        }
        
        std::string errorDescription(int code) const;
        
    public:
        void registerStatement(Statement *stmt);
        void unregisterStatement(Statement *stmt);
        void finilizeAllStatements(bool finilized);

	private:
		Database(): _db(nullptr) {}
        
    private:
        sqlite3 *_db;
        
        std::list<Statement *> _statements;
    };
}

#endif /* Database_hpp */
