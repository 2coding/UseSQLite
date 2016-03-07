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

#ifndef USQLStatement_hpp
#define USQLStatement_hpp

#include "StdCpp.hpp"
#include "Object.hpp"
#include "Query.hpp"

namespace usql {
    class Cursor : public Query
    {
    public:
        Cursor(const std::string &cmd, DBConnection &con);
        
        bool bind(const std::string &key, int value);
        bool bind(const std::string &key, int64_t value);
        bool bind(const std::string &key, double value);
        bool bind(const std::string &key, const std::string &value, BindType opt = BindType::Copy);
        bool bind(const std::string &key, const void *blob, int count, BindType opt = BindType::Copy);
        
        bool bind(int index, int value);
        bool bind(int index, int64_t value);
        bool bind(int index, double value);
        bool bind(int index, const std::string &value, BindType opt = BindType::Copy);
        bool bind(int index, const void *blob, int count, BindType opt = BindType::Copy);
        
        bool exec();
    };
}

#endif /* USQLStatement_hpp */
