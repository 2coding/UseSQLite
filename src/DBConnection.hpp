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

#ifndef DBConnection_hpp
#define DBConnection_hpp

#include "StdCpp.hpp"
#include "Result.hpp"
#include "Query.hpp"
#include "Cursor.hpp"
#include "Function.hpp"

namespace usql {
    class Statement;
    
    class DBConnection : public NoCopyable
    {
    public:
        DBConnection(const std::string &filename);
        virtual ~DBConnection();
        
        Result open();
        Result open(int flags);
        bool isOpenning() const {
            return _db != nullptr;
        }
        
        Result close();
        
        inline sqlite3 *db() {
            return _db;
        }
        
    public:
        Result exec(const std::string &cmd);
        
        Result beginTransaction(TransactionType type);
        Result commit();
        Result rollback();
        Result transaction(TransactionType type, tr1::function<bool()> action);
        
        //tables
        struct ColumnInfo
        {
            std::string name;
            std::string type;
            bool nullable = false;
            std::string defaultValue;
        };
        
        struct TableInfo
        {
            std::string name;
            std::vector<std::string> primaryKeys;
            std::vector<ColumnInfo> columndefs;
        };
        
        bool tableExists(const std::string &tablename, const std::string &schema = "");
        std::vector<std::string> allTables(const std::string &schema = "");
        TableInfo tableInfo(const std::string &name, const std::string &schema = "");
        
        //attach or detach database
        struct DatabaseInfo
        {
            std::string name;
            std::string filepath;
            
            DatabaseInfo(const std::string &n, const std::string &f) : name(n), filepath(f) {}
        };
        Result attachDatabase(const std::string &filename, const std::string &schema);
        void detachDatabase(const std::string &schema);
        std::vector<DatabaseInfo> allDatabase();
        
    public:
        Result registerFunction(Function *func);
        
        void unregisterFunction(const std::string name);
        void unregisterAllFunctions();
        
    protected:
        friend class Statement;
        
        void registerStatement(Statement *stmt);
        void unregisterStatement(Statement *stmt);
        void finilizeAllStatements(bool finilized);
        
    private:
        std::string _filename;
        sqlite3 *_db;
        
        std::list<Statement *> _statements;
        
        std::list<std::string> _functions;
    };
}

#endif /* DBConnection_hpp */
