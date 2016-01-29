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
        _connection.exec("create table if not exists use_sqlite_table(a text, b int)");
    }
    virtual void TearDown() {
        USQLCommand cmd("drop table if exists use_sqlite_table", _connection);
        cmd.exeNoQuery();
        _connection.closeSync();
    }
    
protected:
    USQLConnection _connection;
};

TEST_F(USQLTests, fail_on_closed_database)
{
    _connection.close();
    
    USQLCommand cmd("select * from use_sqlite_table", _connection);
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

TEST_F(USQLTests, query_int)
{
    _connection.exec("insert into use_sqlite_table (a, b) values ('hello world', 10)");
    
    USQLCommand cmd("select * from use_sqlite_table", _connection);
    USQLQuery query = cmd.exeQuery();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
    EXPECT_FALSE(query.next());
}
