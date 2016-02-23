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
    EXPECT_TRUE(connection.isOpenning());
    EXPECT_EQ(SQLITE_OK, connection.lastErrorCode());
    connection.setLastErrorCode(SQLITE_BUSY);
    EXPECT_TRUE(connection.close());
    
    EXPECT_FALSE(connection.isOpenning());
    EXPECT_EQ(SQLITE_OK, connection.lastErrorCode());
    
    connection.open();
    EXPECT_TRUE(connection.isOpenning());
    connection.setLastErrorCode(SQLITE_BUSY);
    
    connection.close();
    EXPECT_FALSE(connection.isOpenning());
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
        dropTable();
        createTable();
        clearTable();
    }
    virtual void TearDown() {
        clearTable();
        dropTable();
        _connection.close();
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
    EXPECT_TRUE(insertRow("row 1", 10, 12.3));
    
    std::string cmd("select * from use_sqlite_table");
    EXPECT_TRUE(_connection.exec(cmd));
    
    USQLQuery query(cmd, _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(4, query.columnCount());
    EXPECT_EQ("row 1", query.textForName("a"));
    EXPECT_EQ(10, query.intForName("b"));
    
    EXPECT_TRUE(_connection.close());
    EXPECT_FALSE(_connection.exec(cmd));
    USQLQuery failed(cmd, _connection);
    EXPECT_EQ(0, failed.columnCount());
    EXPECT_EQ(USQL_ERROR_TEXT, failed.textForName("a"));
    EXPECT_EQ(USQL_ERROR_INTEGER, failed.intForName("b"));
}

TEST_F(USQLTests, fail_on_bad_statement)
{
    EXPECT_FALSE(_connection.exec("bla bla bla"));
    EXPECT_NE(SQLITE_OK, _connection.lastErrorCode());
}

TEST_F(USQLTests, success_exe_sql)
{
    std::string cmd("select * from use_sqlite_table");
    EXPECT_TRUE(_connection.exec(cmd));
    EXPECT_EQ(SQLITE_OK, _connection.lastErrorCode());
    
    EXPECT_TRUE(_connection.exec(cmd));
    EXPECT_EQ(SQLITE_OK, _connection.lastErrorCode());
}

TEST_F(USQLTests, query_on_closed_database)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_TRUE(query.reset());
    
    EXPECT_TRUE(_connection.close());
    EXPECT_FALSE(query.next());
    EXPECT_FALSE(query.reset());
}

TEST_F(USQLTests, query_reset)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_FALSE(query.next());
    
    EXPECT_TRUE(query.reset());
    EXPECT_TRUE(query.next());
}

TEST_F(USQLTests, query_column)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    
    EXPECT_EQ(4, query.columnCount());
    EXPECT_EQ(0, query.columnIndexForName("a"));
    EXPECT_EQ(USQL_INVALID_COLUMN_INDEX, query.columnIndexForName("A"));
    EXPECT_EQ(1, query.columnIndexForName("b"));
    EXPECT_EQ(2, query.columnIndexForName("c"));
    
    EXPECT_TRUE(query.availableIndex(0));
    EXPECT_TRUE(query.availableIndex(3));
    EXPECT_FALSE(query.availableIndex(-1));
    EXPECT_FALSE(query.availableIndex(4));
}

TEST_F(USQLTests, query_column_type)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    USQLQuery query("select * from use_sqlite_table", _connection);
    
    EXPECT_TRUE(query.next());
    
    EXPECT_EQ(USQLColumnType::USQLText, query.typeForName("a"));
    EXPECT_EQ(USQLColumnType::USQLInteger, query.typeForName("b"));
    EXPECT_EQ(USQLColumnType::USQLFloat, query.typeForName("c"));
    EXPECT_EQ(USQLColumnType::USQLNull, query.typeForName("d"));
    EXPECT_EQ(USQLColumnType::USQLInvalidType, query.typeForName("A"));
}

TEST_F(USQLTests, query_int)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
    EXPECT_EQ(10, query.int64ForName("b"));
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("a"));
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_text)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(USQL_ERROR_TEXT, query.textForName("b"));
    EXPECT_EQ("hello world", query.textForName("a"));
    EXPECT_EQ(USQL_ERROR_TEXT, query.textForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_double)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(USQL_ERROR_FLOAT, query.floatForName("b"));
    EXPECT_EQ(USQL_ERROR_FLOAT, query.floatForName("a"));
    EXPECT_EQ(12.3, query.floatForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_close)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    USQLQuery query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
    
    query.close();
    EXPECT_FALSE(query.next());
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("b"));
    
    query.reset();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
}

TEST_F(USQLTests, statement_bind)
{
    USQLStatement stmt("insert into use_sqlite_table (a, b, c, d) values (:a, :b, :c, :d)", _connection);
    const std::string tvalue = "bind hello world";
    int ivalue = 123;
    double rvalue = 111.23;
    const std::string bvalue = "binary data";
    EXPECT_TRUE(stmt.bind(":a", tvalue));
    EXPECT_TRUE(stmt.bind(":b", ivalue));
    EXPECT_TRUE(stmt.bind(":c", rvalue));
    EXPECT_TRUE(stmt.bind(":d", bvalue.data(), (int)bvalue.size()));
    EXPECT_TRUE(stmt.exec());
    
    USQLStatement query("select * from use_sqlite_table where a = ? and b = ? and c = ? and d = ?", _connection);
    EXPECT_TRUE(query.bind(1, tvalue));
    EXPECT_TRUE(query.bind(2, ivalue));
    EXPECT_TRUE(query.bind(3, rvalue));
    EXPECT_TRUE(query.bind(4, bvalue.data(), (int)bvalue.size()));
    EXPECT_TRUE(query.next());
}

TEST_F(USQLTests, connnection_transaction)
{
    USQLQuery query("select count(*) as row_count from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(0, query.intForColumnIndex(0));
    query.close();
    
    _connection.transaction(USQLTransactionType::USQLDeferred
                            , [this]()->bool{
                                this->insertRow("hello world", 10, 10.12);
                                this->insertRow("row 2", 110, 12.22);
                                return false;
                            });
    query.reset();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(0, query.intForColumnIndex(0));
    query.close();
    
    _connection.transaction(USQLTransactionType::USQLDeferred
                            , [this]()->bool{
                                this->insertRow("hello world", 10, 10.12);
                                this->insertRow("row 2", 110, 12.22);
                                return true;
                            });
    query.reset();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(2, query.intForColumnIndex(0));
    query.close();
}

#pragma mark - extension tests
class USQLExtTests : public testing::Test
{
public:
    USQLExtTests() : _connection(_db) {}
    
protected:
    virtual void SetUp() {
        _connection.open();
        _connection.exec("drop table if exists test_table_name");
    }
    virtual void TearDown() {
        _connection.exec("drop table if exists test_table_name");
        _connection.close();
    }
    
    USQLConnection _connection;
    std::string _testTablename = "test_table_name";
};

TEST_F(USQLExtTests, tablename_check)
{
    EXPECT_TRUE(USQLTableCommand::checkTablename("test_table_name"));
    EXPECT_FALSE(USQLTableCommand::checkTablename("sqlite_test_table_name"));
}

TEST_F(USQLExtTests, create_table)
{
    std::map<USQLColumnConstraint, std::string> opta;
    opta[USQLColumnConstraint::PrimaryKey] = "";
    
    std::map<USQLColumnConstraint, std::string> optb;
    optb[USQLColumnConstraint::NotNull] = "";
    optb[USQLColumnConstraint::Unique] = "";
    optb[USQLColumnConstraint::Check] = "(b > 100)";
    
    std::map<USQLColumnConstraint, std::string> optc;
    optc[USQLColumnConstraint::NotNull] = "";
    optc[USQLColumnConstraint::Default] = "'hello world'";
    optc[USQLColumnConstraint::Collate] = "NOCASE";
    
    std::vector<std::string> unique;
    unique.push_back("a");
    unique.push_back("c");
    
    auto cmd = USQLTableCommand::create(_testTablename);
    cmd.temp(false).createIfNotExist(true)
    .columnDef("a", "int", opta)
    .columnDef("b", "int", optb)
    .columnDef("c", "text", optc)
    .tableConstraintCheck("(a + b < 200)")
    .tableConstraintUnique(unique)
    .withoutRowId(false);
    
    EXPECT_TRUE(_connection.exec(cmd.command()));
    EXPECT_TRUE(_connection.exec("insert into test_table_name (a, b) values (1, 101)"));
    EXPECT_FALSE(_connection.exec("insert into test_table_name (a, b) values (2, 10)"));
    EXPECT_FALSE(_connection.exec("insert into test_table_name (a, b) values (3, 197)"));
    
    USQLQuery query("select * from test_table_name where c = 'HELLO WORLD'", _connection);
    EXPECT_TRUE(query.next());
}