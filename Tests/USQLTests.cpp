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

#include "USQLTests.hpp"
#include "USQL.hpp"
#include <sstream>
using namespace usqlite;

static const char *_db = "/tmp/usqlite.db";

int usqlite_test_run(int argc, const char **argv) {
    testing::InitGoogleTest(&argc, const_cast<char **>(argv));
    return RUN_ALL_TESTS();
}

#pragma mark - global test
TEST(usqlite_tests, open_close_database)
{
    USQLConnection connection(_db);
    EXPECT_TRUE(connection.open());
    EXPECT_EQ(SQLITE_OK, connection.lastErrorCode());
    connection.setLastErrorCode(SQLITE_BUSY);
    EXPECT_TRUE(connection.closeSync());
    EXPECT_EQ(SQLITE_OK, connection.lastErrorCode());
    
    connection.open();
    connection.setLastErrorCode(SQLITE_BUSY);
    connection.close();
    EXPECT_EQ(SQLITE_OK, connection.lastErrorCode());
}

#pragma mark - sqlite base tests
class USQLTests : public testing::Test
{
public:
    USQLTests() : _connection(_db) {}
    
protected:
    static void SetUpTestCase() {
        
    }
    
    static void TearDownTestCase() {
        
    }
    
    virtual void SetUp() {
        _connection.open();
        createTable();
    }
    virtual void TearDown() {
        dropTable();
        _connection.closeSync();
    }
    
    bool createTable() {
        return _connection.exec("create table if not exists use_sqlite_table(a text, b int, c real, d blob)");
    }
    
    bool dropTable() {
        return _connection.exec("drop table if exists use_sqlite_table");
    }
    
    bool insertRow(const std::string &a, int b, double c) {
        std::stringstream ss;
        ss<<"insert into use_sqlite_table (a, b, c) values ('"<<a<<"', "<<b<<", "<<c<<")";
        std::string cmd = ss.str();
        return _connection.exec(cmd);
    }
    
    bool clearTable() {
        return _connection.exec("delete from use_sqlite_table");
    }
    
protected:
    USQLConnection _connection;
};

TEST_F(USQLTests, fail_on_closed_database)
{
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(cmd.exeNoQuery());
    EXPECT_TRUE(_connection.closeSync());
    EXPECT_FALSE(cmd.exeNoQuery());
}

TEST_F(USQLTests, fail_on_bad_statement)
{
    USQLCommand cmd("bla bla bla", _connection);
    EXPECT_FALSE(cmd.exeNoQuery());
    EXPECT_NE(SQLITE_OK, _connection.lastErrorCode());
}

TEST_F(USQLTests, success_exe_sql)
{
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(cmd.exeNoQuery());
    EXPECT_EQ(SQLITE_OK, _connection.lastErrorCode());
    
    EXPECT_TRUE(cmd.exeNoQuery());
    EXPECT_EQ(SQLITE_OK, _connection.lastErrorCode());
}

TEST_F(USQLTests, query_on_closed_database)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    USQLQuery query = cmd.exeQuery();
    EXPECT_TRUE(query.next());
    EXPECT_TRUE(query.reset());
    
    EXPECT_TRUE(_connection.closeSync());
    EXPECT_FALSE(query.next());
    EXPECT_FALSE(query.reset());
}

TEST_F(USQLTests, query_reset)
{
    EXPECT_TRUE(clearTable());
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    USQLQuery query = cmd.exeQuery();
    
    EXPECT_TRUE(query.next());
    EXPECT_FALSE(query.next());
    
    EXPECT_TRUE(query.reset());
    EXPECT_TRUE(query.next());
}

TEST_F(USQLTests, query_int)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    USQLQuery query = cmd.exeQuery();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("a"));
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_text)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    USQLQuery query = cmd.exeQuery();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(USQL_ERROR_TEXT, query.textForName("b"));
    EXPECT_EQ("hello world", query.textForName("a"));
    EXPECT_EQ(USQL_ERROR_TEXT, query.textForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_double)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    USQLQuery query = cmd.exeQuery();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(USQL_ERROR_FLOAT, query.doubleForName("b"));
    EXPECT_EQ(USQL_ERROR_FLOAT, query.doubleForName("a"));
    EXPECT_EQ(12.3, query.doubleForName("c"));
    EXPECT_FALSE(query.next());
}
