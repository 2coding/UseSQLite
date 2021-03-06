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

#ifndef Query_hpp
#define Query_hpp

#include "Cursor.hpp"

namespace usql {
    class Query : public Cursor
    {
    public:
        //using Cursor::Cursor;
		Query(const std::string &cmd, Connection &db): Cursor(cmd, db) {}
        
        Result next();
        Result reset();
        
        int columnCount() const;
        bool availableIndex(int idx) const;
        int columnIndexForName(const std::string &name) const;
        
        ColumnType typeForName(const std::string &name) const;
        ColumnType typeForColumn(int i) const;
        
        int intForName(const std::string &name);
        int intForColumnIndex(int idx);
        
        bool booleanForName(const std::string &name);
        bool booleanForColumnIndex(int idx);
        
        int64_t int64ForName(const std::string &name);
        int64_t int64ForColumnIndex(int idx);
        
        std::string textForName(const std::string &name);
        std::string textForColumnIndex(int idx);
        
        double floatForName(const std::string &name);
        double floatForColumnIndex(int idx);
        
        std::time_t datetimeForName(const std::string &name);
        std::time_t datetimeForColumnIndex(int idx);
        
    protected:
        const unsigned char *cstrForColumnIndex(int idx);
    };
}

#endif /* Query_hpp */
