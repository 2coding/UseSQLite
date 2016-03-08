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

#ifndef Statement_hpp
#define Statement_hpp

#include "StdCpp.hpp"
#include "USQLDefs.hpp"
#include "Utils.hpp"
#include "Object.hpp"
#include "Result.hpp"

namespace usql {
    class DBConnection;
    class Statement : public NoCopyable
    {
    public:
        Statement(const std::string &cmd, DBConnection *db);
        ~Statement();
        
        std::string command() const {
            return _command;
        }
        
        Result reset();
        Result step();
        Result query();
        void finilize();
        
        inline sqlite3_stmt *statement() {
            return _stmt;
        }
        
        inline int columnCount() const {
            return static_cast<int>(_columns.size());
        }
        int columnIndexForName(const std::string &name) const;
        ColumnType typeForColumnIndex(int i) const;
        
        template<class Type>
        Type staticValueForColumnIndex(int idx, ColumnType expect, Type (*fn)(sqlite3_stmt *, int), Type err) {
            if (!Statement::safeTypeCast(typeForColumnIndex(idx), expect)) {
                return err;
            }
            
            if (!fn) {
                return err;
            }
            
            return fn(_stmt, idx);
        }
        
        int parameterIndexForName(const std::string &name) const;
        
        template<class... TArgs>
        Result bindName(const std::string &name, int (*func)(sqlite3_stmt *, int, TArgs...), TArgs... args) {
            int i = parameterIndexForName(name);
            return bindIndex(i, func, args...);
        }
        
        template<class... TArgs>
        Result bindIndex(int i, int (*func)(sqlite3_stmt *, int, TArgs...), TArgs... args) {
            if (i <= USQL_INVALID_PARAMETER_INDEX || i > _parametersCount || !_stmt) {
                return Result::error();
            }
            
            Result ret = reset();
            if (!ret) {
                return ret;
            }
            
            return Result(func(_stmt, i, args...), connection());
        }
        
    private:
        void initColumnInfo();
        void clearColumnInfo() {
            _columns.clear();
            _columnTypes.clear();
        }
        
        void initParameters();
        void clearParameters() {
            _nameParameters.clear();
            _parametersCount = 0;
        }
        
        ColumnType typeForColumn(int i);
        
        static bool safeTypeCast(ColumnType actual, ColumnType expect) {
            return actual == expect;
        }
        
        Result prepare();
        
        sqlite3 *connection();
        
    private:
        const std::string _command;
        sqlite3_stmt *_stmt;
        DBConnection *_db;
        
        std::map<std::string, int> _columns;
        std::vector<ColumnType> _columnTypes;
        
        std::map<std::string, int> _nameParameters;
        int _parametersCount;
    };
}

#endif /* Statement_hpp */
