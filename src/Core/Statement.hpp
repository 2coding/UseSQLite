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
#include "Database.hpp"

namespace usql {
//#if !_USQL_TEMPLATE_VARIABLE_PARAMETERS_ENABLE
	_USQL_ENUM_CLASS_DEF(BindValueType) {
		NoneValue,
			IntValue,
			Int64Value,
			DoubleValue,
			TextValue,
			BlobValue
	};

	struct BindValue{
		union {
			int i;
			int64_t i64;
			double d;
			const char *str;
			const void *blob;
		}v;

		int count;
		sqlite3_destructor_type destructor;
		BindValueType type;

		BindValue(int i) {
			init();

			v.i = i;
			type = _USQL_ENUM_VALUE(BindValueType, IntValue);
		}

		BindValue(int64_t i64) {
			init();

			v.i64 = i64;
			type = _USQL_ENUM_VALUE(BindValueType, Int64Value);
		}

		BindValue(double d) {
			init();

			v.d = d;
			type = _USQL_ENUM_VALUE(BindValueType, DoubleValue);
		}

		BindValue(const char *str, int count, sqlite3_destructor_type destructor) {
			init();

			v.str = str;
			count = count;
			destructor = destructor;

			type = _USQL_ENUM_VALUE(BindValueType, TextValue);
		}

		BindValue(const void *blob, int count, sqlite3_destructor_type destructor) {
			init();

			v.blob = blob;
			count = count;
			destructor = destructor;

			type = _USQL_ENUM_VALUE(BindValueType, BlobValue);
		}

	private:
		void init() {
			type = _USQL_ENUM_VALUE(BindValueType, NoneValue);
			memset(&v, 0, sizeof(v));
			count = -1;
			destructor = nullptr;
		}
	};

//#endif
    class Statement : public NoCopyable
    {
    public:
        Statement(const std::string &cmd, _WeakDatabase db);
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
        ColumnType typeForColumnIndex(size_t i) const;
        
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

//#if _USQL_TEMPLATE_VARIABLE_PARAMETERS_ENABLE
//        template<class... TArgs>
//        Result bindName(const std::string &name, int (*func)(sqlite3_stmt *, int, TArgs...), TArgs... args) {
//            int i = parameterIndexForName(name);
//            return bindIndex(i, func, args...);
//        }
//        
//        template<class... TArgs>
//        Result bindIndex(int i, int (*func)(sqlite3_stmt *, int, TArgs...), TArgs... args) {
//            if (i <= USQL_INVALID_PARAMETER_INDEX || i > _parametersCount || !_stmt) {
//                return Result::error();
//            }
//            
//            Result ret = reset();
//            if (!ret) {
//                return ret;
//            }
//            
//            return Result(func(_stmt, i, args...), _db);
//        }
//#else
		Result bindName(const std::string &name, const BindValue &value) {
			int i = parameterIndexForName(name);
			return bindIndex(i, value);
		}

		Result bindIndex(int i, const BindValue &value) {
			if (i <= USQL_INVALID_PARAMETER_INDEX || i > _parametersCount || !_stmt) {
				return Result::error();
			}

			Result ret = reset();
			if (!ret) {
				return ret;
			}

			if (value.type == _USQL_ENUM_VALUE(BindValueType, IntValue)) {
				return Result(sqlite3_bind_int(_stmt, i, value.v.i), _db);
			}
			else if (value.type == _USQL_ENUM_VALUE(BindValueType, Int64Value)) {
				return Result(sqlite3_bind_int64(_stmt, i, value.v.i64), _db);
			}
			else if (value.type == _USQL_ENUM_VALUE(BindValueType, DoubleValue)) {
				return Result(sqlite3_bind_double(_stmt, i, value.v.d), _db);
			}
			else if (value.type == _USQL_ENUM_VALUE(BindValueType, TextValue)) {
				return Result(sqlite3_bind_text(_stmt, i, value.v.str, value.count, value.destructor), _db);
			}
			else if (value.type == _USQL_ENUM_VALUE(BindValueType, BlobValue)) {
				return Result(sqlite3_bind_blob(_stmt, i, value.v.blob, value.count, value.destructor), _db);
			}

			return Result(false);
		}
//#endif
        
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
        
    private:
        const std::string _command;
        sqlite3_stmt *_stmt;
        _WeakDatabase _db;
        
        std::map<std::string, int> _columns;
        std::vector<ColumnType> _columnTypes;
        
        std::map<std::string, int> _nameParameters;
        int _parametersCount;
    };
}

#endif /* Statement_hpp */
