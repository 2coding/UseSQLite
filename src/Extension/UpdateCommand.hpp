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

#ifndef UpdateCommand_hpp
#define UpdateCommand_hpp

#include "ExprCommand.hpp"

namespace usql {
    class UpdateCommand : public ExprCommand<UpdateCommand>
    {
    public:
        //using ExprCommand::ExprCommand;
		UpdateCommand(const std::string &tablename): ExprCommand(tablename) {}
        
        virtual std::string command() const override;
        
        template<class TValue>
        UpdateCommand &value(const std::string &name, TValue v) {
            if (name.empty()) {
                return *this;
            }
            
            std::stringstream buf;
            const std::string str = tr1::is_convertible<TValue, const char *>::value ? "'" : "";
            buf<<str<<v<<str;
            return expr(name, buf.str());
        }
        
        UpdateCommand &datetimeNow(const std::string &name);
        UpdateCommand &timeNow(const std::string &name);
        
        UpdateCommand &expr(const std::string &name, const std::string &e);
        
    private:
        std::map<std::string, std::string> _column;
    };
}

#endif /* UpdateCommand_hpp */
