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

#include <iostream>
#include "USQL.hpp"
using namespace usql;

#ifdef _MSC_VER 
static const char *bdfile = "example.db";
#else
static const char *bdfile = "/tmp/example.db";
#endif

int main(int argc, const char * argv[]) {
    //open connection
    Connection db(bdfile);
    db.open();
    db.exec("create table if not exists table_name(a int, b real, c text)");
    db.exec("insert into table_name (a, b, c) values (10, 11.2, 'hello world')");
    
    //Transaction
    db.transaction(_USQL_ENUM_VALUE(TransactionType, Deferred), [](Connection &con)->bool{
        con.exec("insert into table_name (a, b, c) values (1, 1.1, 'step one')");
        con.exec("insert into table_name (a, b, c) values (2, 2.2, 'step two')");
        return true;
    });
    
    //query
    Query query("select * from table_name", db);
    while(query.next()) {
        query.intForName("a");
        query.floatForName("b");
        query.textForName("c");
    }
    
    //bind
    Cursor cursor("insert into table_name (a, b, c) values (:a, :b, :c)", db);
    cursor.bind(":a", 10);
    cursor.bind(":b", 11.2);
    cursor.bind(":c", "hello world");
    cursor.exec();
    
#if _USQL_SQLITE_CREATE_FUNCTION_V2_ENABLE
    //create function
    Function *obj = Function::create("usql_string_len");
    obj->setFunction([](sqlite3_context* context, std::vector<sqlite3_value *> &argv){
        if (argv.size() >= 1) {
            auto text = sqlite3_value_text(argv[0]);
            if (text) {
                int len = (int)std::strlen((const char *)text);
                sqlite3_result_int(context, len);
                return ;
            }
        }
        
        sqlite3_result_int(context, -1);
    });
    obj->setArgumentCount(1);
    obj->deterministic = true;
    db.registerFunction(obj);
    
    //create aggregate function
    AggregateFunction *agg = AggregateFunction::create("max_text_len");
    static int maxLen = -1;
    agg->setFunction([&](sqlite3_context* context, std::vector<sqlite3_value *> &argv){
        if (argv.size() >= 1) {
            auto text = sqlite3_value_text(argv[0]);
            if (text) {
                maxLen = std::max(maxLen, (int)std::strlen((const char *)text));
            }
        }
    });
    agg->setFinalFunction([](sqlite3_context *context){
        sqlite3_result_int(context, maxLen);
    });
    db.registerFunction(agg);
#endif
    
    db.close();
    
    return 0;
}
