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

#include "TableCommand.hpp"

namespace usql {
    bool TableCommand::checkTablename(const std::string tablename) {
        if (tablename.empty()) {
            return false;
        }
        
        return tablename.find("sqlite_") != 0;
    }
    
    TableCommand::CreateTableCommand TableCommand::create(const std::string &tablename) {
        return TableCommand::CreateTableCommand(tablename);
    }
    
    TableCommand::DropTableCommand TableCommand::drop(const std::string &tablename) {
        return TableCommand::DropTableCommand(tablename);
    }
    
    std::string TableCommand::rename(const std::string &oldname, const std::string &newname, const std::string &schema) {
        if (!checkTablename(oldname) || !checkTablename(newname)) {
            return "";
        }
        
        std::stringstream buf;
        buf<<"ALTER TABLE ";
        if (!schema.empty()) {
            buf<<schema<<".";
        }
        
        buf<<oldname<<" RENAME TO "<<newname;
        return buf.str();
    }
    
    TableCommand::AlterTableCommand TableCommand::alter(const std::string &tablename) {
        return TableCommand::AlterTableCommand(tablename);
    }
    
#pragma mark - column-def
    std::string TableCommand::_column::opt(const std::map<ColumnConstraint, std::string> &c) {
        std::stringstream ss;
        for (auto iter = c.begin(); iter != c.end(); ++iter) {
            if (iter != c.begin()) {
                ss<<" ";
            }
            
            auto type = iter->first;
            switch (type) {
                case _USQL_ENUM_VALUE(ColumnConstraint, PrimaryKey):
                    ss<<"PRIMARY KEY";
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, PrimaryKeyAsc):
                    ss<<"PRIMARY KEY ASC";
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, PrimaryKeyDesc):
                    ss<<"PRIMARY KEY DESC";
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, Autoincrement):
                    ss<<"AUTOINCREMENT";
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, NotNull):
                    ss<<"NOT NULL";
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, Unique):
                    ss<<"UNIQUE";
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, Check):
                    if (!iter->second.empty()) {
                        ss<<"CHECK"<<iter->second;
                    }
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, Default):
                    if (!iter->second.empty()) {
                        ss<<"DEFAULT "<<iter->second;
                    }
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, ForeignKey):
                    if (!iter->second.empty()) {
                        ss<<"FOREIGN KEY "<<iter->second;
                    }
                    break;
                    
                case _USQL_ENUM_VALUE(ColumnConstraint, Collate):
                    if (!iter->second.empty()) {
                        ss<<"COLLATE "<<iter->second;
                    }
                    break;
            }
        }
        
        return ss.str();
    }
    
#pragma mark - CREATE TABLE
    TableCommand::CreateTableCommand &TableCommand::CreateTableCommand::columnDef(const std::string &name, const std::string &type, const std::string &constraint) {
        if (name.empty() || type.empty()) {
            return *this;
        }
        
        _columnDef[name] = _column(name, type, constraint);
        return *this;
    }
    
    TableCommand::CreateTableCommand &TableCommand::CreateTableCommand::columnDef(const std::string &name, const std::string &type, const std::map<ColumnConstraint, std::string> &constraint) {
        return columnDef(name, type, TableCommand::_column::opt(constraint));
    }
    
    TableCommand::CreateTableCommand &TableCommand::CreateTableCommand::tableConstraintPrimaryKey(const std::vector<std::string> columns) {
        if (!columns.empty()) {
            _tableConstraint[_USQL_ENUM_VALUE(ColumnConstraint, PrimaryKey)] = "PRIMARY KEY " + joinString(columns);
        }
        
        return *this;
    }
    
    TableCommand::CreateTableCommand &TableCommand::CreateTableCommand::tableConstraintUnique(const std::vector<std::string> columns) {
        if (!columns.empty()) {
            _tableConstraint[_USQL_ENUM_VALUE(ColumnConstraint, Unique)] = "UNIQUE " + joinString(columns);
        }
        
        return *this;
    }
    
    TableCommand::CreateTableCommand &TableCommand::CreateTableCommand::tableConstraintCheck(const std::string &expr) {
        if (!expr.empty()) {
            _tableConstraint[_USQL_ENUM_VALUE(ColumnConstraint, Check)] = "CHECK " + expr;
        }
        
        return *this;
    }
    
    TableCommand::CreateTableCommand &TableCommand::CreateTableCommand::tableConstraintForeignKey(const std::vector<std::string> columns, const std::string &clause) {
        if (!columns.empty()) {
            std::string cmd = "FOREIGN KEY " + joinString(columns) + " " + clause;
            _tableConstraint[_USQL_ENUM_VALUE(ColumnConstraint, ForeignKey)] = cmd;
        }
        
        return *this;
    }
    
    std::string TableCommand::CreateTableCommand::command() const {
        std::stringstream ss;
        ss<<"CREATE ";
        
        if (_temp) {
            ss<<"TEMP ";
        }
        
        ss<<"TABLE ";
        if (_createIfNotExist) {
            ss<<"IF NOT EXISTS ";
        }
        
        ss<<tablename();
        
        ss<<" (";
        for (auto iter = _columnDef.begin(); iter != _columnDef.end(); ++iter) {
            auto info = iter->second;
            if (info.invalid()) {
                continue;
            }
            
            if (iter != _columnDef.begin()) {
                ss<<","<<std::endl;
            }

            ss<<info.columndef();
        }
        
        for (auto iter = _tableConstraint.begin(); iter != _tableConstraint.end(); ++iter) {
            ss<<","<<std::endl;
            ss<<iter->second;
        }
        
        ss<<") ";
        if (_withoutRowId) {
            ss<<"WITHOUT ROWID";
        }
        
        return ss.str();
    }
    
#pragma mark - DROP TABLE
    std::string TableCommand::DropTableCommand::command() const {
        std::stringstream ss;
        ss<<"DROP TABLE ";
        
        if (_ifExists) {
            ss<<"IF EXISTS ";
        }
        
        ss<<tablename();
        return ss.str();
    }

#pragma mark - ALTER TABLE
    std::string TableCommand::AlterTableCommand::command() const {
        if (_columndef.invalid()) {
            return "";
        }
        
        std::stringstream ss;
        ss<<"ALTER TABLE ";
        
        ss<<tablename();
        
        ss<<" ADD "<<_columndef.columndef();
        return ss.str();
    }

}