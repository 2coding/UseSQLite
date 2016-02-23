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

#ifndef USQLTableCommand_hpp
#define USQLTableCommand_hpp

#include "USQLCommand.hpp"

namespace usqlite {
    class USQLTableCommand : public USQLCommand
    {
    public:
        class CreateTableCommand : public USQLCommand
        {
        public:
            CreateTableCommand(const std::string &tablename);
            
            CreateTableCommand &createIfNotExist(bool b) {
                _createIfNotExist = b;
                return *this;
            }
            
            CreateTableCommand &schema(const std::string &name) {
                _schema = name;
                return *this;
            }
            
            CreateTableCommand &withoutRowId(bool b) {
                _withoutRowId = b;
                return *this;
            }
            
            CreateTableCommand &temp(bool b) {
                _temp = b;
                return *this;
            }
            
            CreateTableCommand &columnDef(const std::string &name, const std::string &type, const std::string &constraint = "");
            CreateTableCommand &columnDef(const std::string &name, const std::string &type, const std::map<USQLColumnConstraint, std::string> &constraint);
            
            CreateTableCommand &tableConstraintPrimaryKey(const std::vector<std::string> columns);
            CreateTableCommand &tableConstraintUnique(const std::vector<std::string> columns);
            CreateTableCommand &tableConstraintCheck(const std::string &expr);
            CreateTableCommand &tableConstraintForeignKey(const std::vector<std::string> columns, const std::string &clause);
            
            virtual std::string command() const override;
            
        private:
            std::string joinString(const std::vector<std::string> v) {
                std::stringstream ss;
                ss<<"(";
                for (auto iter = v.begin(); iter != v.end(); ++iter) {
                    if (iter != v.begin()) {
                        ss<<", ";
                    }
                    
                    ss<<*iter;
                }
                
                ss<<")";
                return ss.str();
            }
          
        private:
            struct _column
            {
                std::string name;
                std::string type;
                std::string constraint;
            };
            
        private:
            std::string _schema = "";
            std::string _tablename;
            
            bool _temp = false;
            bool _withoutRowId = false;
            bool _createIfNotExist = true;
            
            std::map<std::string, _column> _columnDef;
            std::map<USQLColumnConstraint, std::string> _tableConstraint;
        };
        
    public:
        static bool checkTablename(const std::string tablename);
        
        static CreateTableCommand create(const std::string &tablename);
    };
}

#endif /* USQLTableCommand_hpp */
