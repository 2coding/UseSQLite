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
#pragma mark - column-def
    private:
        struct _column
        {
            std::string name = "";
            std::string type = "";
            std::string constraint = "";
            
            _column() {}
            _column(const std::string &n, const std::string &t, const std::string &c) :name(n), type(t), constraint(c) {}
            _column(const std::string &n, const std::string &t, const std::map<USQLColumnConstraint, std::string> &c) : name(n), type(t), constraint(opt(c)) {}
            
            std::string columndef() const {
                if (invalid()) {
                    return "";
                }
                
                std::stringstream ss;
                ss<<name<<" "<<type;
                if (!constraint.empty()) {
                    ss<<" "<<constraint;
                }
                
                return ss.str();
            }
            
            bool invalid() const {
                return name.empty() || type.empty();
            }
            
            static std::string opt(const std::map<USQLColumnConstraint, std::string> &opt);
        };
        

#pragma mark - CREATE TABLE
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
            std::string _schema = "";
            std::string _tablename;
            
            bool _temp = false;
            bool _withoutRowId = false;
            bool _createIfNotExist = true;
            
            std::map<std::string, _column> _columnDef;
            std::map<USQLColumnConstraint, std::string> _tableConstraint;
        };
        
#pragma mark - DROP TABLE
        class DropTableCommand : public USQLCommand
        {
        public:
            DropTableCommand(const std::string &tablename);
            
            DropTableCommand &schema(const std::string &name) {
                _schema = name;
                return *this;
            }
            
            DropTableCommand &ifExists(bool b) {
                _ifExists = b;
                return *this;
            }
            
            virtual std::string command() const override;
            
        private:
            std::string _schema = "";
            std::string _tablename;
            
            bool _ifExists = true;
        };
        
#pragma mark - ALTER TABLE
        class AlterTableCommand : public USQLCommand
        {
        public:
            AlterTableCommand(const std::string &tablename) : _tablename(tablename) {}
            
            AlterTableCommand &columnDef(const std::string &name, const std::string &type, const std::string &constraint = "") {
                if (name.empty() || type.empty()) {
                    return *this;
                }
                
                _columndef.name = name;
                _columndef.type = type;
                _columndef.constraint = constraint;
                return *this;
            }
            
            AlterTableCommand &columnDef(const std::string &name, const std::string &type, const std::map<USQLColumnConstraint, std::string> &constraint) {
                return columnDef(name, type, _column::opt(constraint));
            }
            
            AlterTableCommand &schema(const std::string &name) {
                _schema = name;
                return *this;
            }
            
            virtual std::string command() const override;
            
        private:
            std::string _schema = "";
            std::string _tablename;
            
            _column _columndef;
        };
        
#pragma mark - table command
    public:
        static bool checkTablename(const std::string tablename);
        
        static CreateTableCommand create(const std::string &tablename);
        static DropTableCommand drop(const std::string &tablename);
        static AlterTableCommand alter(const std::string &tablename);
        
        static std::string rename(const std::string &oldname, const std::string &newname, const std::string &schema = "");
    };
}

#endif /* USQLTableCommand_hpp */
