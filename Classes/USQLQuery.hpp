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

#ifndef USQLQuery_hpp
#define USQLQuery_hpp

#include <string>
#include <map>

#define USQL_ERROR_INTEGER 0
#define USQL_ERROR_TEXT 0
#define USQL_ERROR_FLOAT 0.0f
#define USQL_ERROR_BLOB 0


namespace usqlite {
    class USQLSatement;
    class USQLQuery
    {
    public:
        USQLQuery(USQLSatement *stmt);
        USQLQuery(const USQLQuery &other);
        virtual ~USQLQuery();
        
        bool next();
        bool reset();
        
        int intForName(const std::string &name);
        
    private:
        bool initNameColumn();
        int hasName(const std::string &name);
        
    private:
        USQLQuery &operator=(const USQLQuery &other) {
            return *this;
        }
        
    private:
        USQLSatement *_stmt;
        std::map<std::string, int> _columns;
    };
}

#endif /* USQLQuery_hpp */