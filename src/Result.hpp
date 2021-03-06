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

#ifndef Result_hpp
#define Result_hpp

#include "StdCpp.hpp"
#include "Object.hpp"
#include "Utils.hpp"
#include "Database.hpp"

namespace usql {
    class Result : public Object
    {
    protected:
        _USQL_ENUM_CLASS_DEF(ResultType) {
            Normal,
            Step,
            Query
        };
        
    public:
        static Result error() {
            return Result(SQLITE_ERROR, nullptr);
        }
        
        static Result success() {
            return Result(SQLITE_OK, nullptr);
        }
        
        static Result step(int c, _WeakDatabase db) {
            return Result(c, db, _USQL_ENUM_VALUE(ResultType, Step));
        }
        
        static Result query(int c, _WeakDatabase db) {
            return Result(c, db, _USQL_ENUM_VALUE(ResultType, Query));
        }
        
        Result(bool res) {
			int code = res ? SQLITE_OK : SQLITE_ERROR;
			init(code, _USQL_SQLITE_ERRSTR(code), _USQL_ENUM_VALUE(ResultType, Normal));
		}
        
        Result(int c, sqlite3 *db, ResultType type = _USQL_ENUM_VALUE(ResultType, Normal)) {
			init(c, (db ? sqlite3_errmsg(db) : _USQL_SQLITE_ERRSTR(c)), type);
		}
        
        Result(int c, _WeakDatabase db, ResultType type = _USQL_ENUM_VALUE(ResultType, Normal)) {
			init(c, db.expired() ? _USQL_SQLITE_ERRSTR(c) :  db.lock()->errorDescription(c), type);
		}
        
        Result(int c, _Database db, ResultType type = _USQL_ENUM_VALUE(ResultType, Normal)) {
			init(c, db->errorDescription(c), type);
		}
        
        Result(int c, const std::string &msg, ResultType type = _USQL_ENUM_VALUE(ResultType, Normal)) {
			init(c, msg, type);
		}
        
        Result(const Result &other) {
			init(other.code(), other.description(), other._type);
		}

        const Result & operator=(const Result &other) {
            if (this == &other) {
                return *this;
            }
            
            _code = other._code;
            _description = other._description;
            _type = other._type;
            return *this;
        }
        
        int code() const {
            return _code;
        }
        
        std::string description() const {
            return _description;
        }
        
        bool isSuccess() const {
            return _type == _USQL_ENUM_VALUE(ResultType, Normal) ? _USQL_OK(_code) 
				: (_type == _USQL_ENUM_VALUE(ResultType, Step) ? _USQL_STEP_OK(_code) : _USQL_QUERY_OK(_code));
        }
        
        operator bool() const {
            return isSuccess();
        }

	private:
		void init(int c, const std::string &msg, ResultType type) {
			_code = c;
			_description = msg;
			_type = type;
		}

    public:
        int _code;
        std::string _description;
        ResultType _type;
    };
}

#endif /* Result_hpp */
