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

#include "USQLSatement.hpp"
#include "USQLConnection.hpp"
#include "USQLDefs.hpp"

namespace usqlite {
    USQLSatement::USQLSatement(const std::string &cmd, USQLConnection &connection)
    : _stmt(nullptr)
    , _command(cmd)
    , _connection(connection){
    }
    
    USQLSatement::~USQLSatement() {
        finilize();
    }
    
    USQLSatement * USQLSatement::create(const std::string &cmd, USQLConnection &connection) {
        USQLSatement *stmt = new USQLSatement(cmd, connection);
        return stmt;
    }
    
    void USQLSatement::finilize() {
        if (!_stmt) {
            return;
        }
        
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
        _connection.unregisterStatement(this);
    }
    
    bool USQLSatement::prepare() {
        if (_stmt) {
            return true;
        }
        
        sqlite3 *db = _connection.db();
        int code = sqlite3_prepare_v2(db, _command.c_str(), static_cast<int>(_command.size() + 1), &_stmt, nullptr);
        if (!USQL_OK(code)) {
            _connection.setLastErrorCode(code);
        }
        else {
            _connection.registerStatement(this);
        }
        
        return USQL_OK(code);
    }
    
    bool USQLSatement::reset() {
        if (_stmt) {
            sqlite3_reset(_stmt);
            return true;
        }
        else {
            return prepare();
        }
    }
    
    bool USQLSatement::step() {
        if (!reset()) {
            return false;
        }
        
        int code = sqlite3_step(_stmt);
        if (!USQL_STEP_OK(code)) {
            _connection.setLastErrorCode(code);
        }
        
        return USQL_STEP_OK(code);
    }
    
    bool USQLSatement::query() {
        if (!_stmt && !reset()) {
            return false;
        }
        
        int code = sqlite3_step(_stmt);
        if (!USQL_STEP_OK(code)) {
            _connection.setLastErrorCode(code);
        }
        
        return USQL_QUERY_OK(code);
    }
}
