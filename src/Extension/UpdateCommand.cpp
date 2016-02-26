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

#include "UpdateCommand.hpp"

namespace usql {
    UpdateCommand &UpdateCommand::expr(const std::string &name, const std::string &e) {
        if (name.empty() || e.empty()) {
            return *this;
        }
        
        _column[name] = e;
        return *this;
    }
    
    UpdateCommand &UpdateCommand::datetimeNow(const std::string &name) {
        return expr(name, "datetime('now', 'localtime')");
    }
    
    UpdateCommand &UpdateCommand::timeNow(const std::string &name) {
        return expr(name, "time('now', 'localtime')");
    }
    
    std::string UpdateCommand::command() const {
        if (_column.size() == 0) {
            return "";
        }
        
        std::stringstream set;
        for (auto iter = _column.begin(); iter != _column.end(); ++iter) {
            if (iter != _column.begin()) {
                set<<", ";
            }
            
            set<<iter->first<<"="<<iter->second;
        }
        
        std::stringstream buf;
        buf<<"UPDATE "<<tablename()<<" SET "<<set.str();
        
        std::string e = exprStr();
        if (!e.empty()) {
            buf<<" "<<e;
        }
        return buf.str();
    }
}