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

#ifndef USQLDefs_hpp
#define USQLDefs_hpp
#include "StdCpp.hpp"

#define USQL_ERROR_INTEGER 0
#define USQL_ERROR_TEXT ""
#define USQL_ERROR_FLOAT (double)0.0f
#define USQL_ERROR_BLOB ""
#define USQL_ERROR_DATATIME (std::time_t)-1

#define USQL_INVALID_COLUMN_INDEX -1
#define USQL_INVALID_PARAMETER_INDEX 0

namespace usql {
    _USQL_ENUM_CLASS_DEF(Encoding) {
        UTF8 = SQLITE_UTF8,
        UTF16LE = SQLITE_UTF16LE,
        UTF16BE = SQLITE_UTF16BE,
        UTF16 = SQLITE_UTF16
    };
    
    _USQL_ENUM_CLASS_DEF(ColumnType) {
        InvalidType,
        Integer,
        Text,
        Float,
        Blob,
        Null,
    };
    
    _USQL_ENUM_CLASS_DEF(TransactionType) {
        Deferred,
        Immediate,
        Exclusive
    };
    
    _USQL_ENUM_CLASS_DEF(ColumnConstraint) {
        PrimaryKey,
        PrimaryKeyAsc,
        PrimaryKeyDesc,
        Autoincrement,
        NotNull,
        Unique,
        Check,
        Default,
        ForeignKey,
        Collate
    };
    
    _USQL_ENUM_CLASS_DEF(BindType) {
        Copy,
        Static
    };
}

#endif /* USQLDefs_hpp */
