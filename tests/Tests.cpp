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

#include "Tests.hpp"
#include "USQL.hpp"
#include <sstream>
#include <cstdio>

using namespace usql;

static const char *_db = "/tmp/usqlite.db";

static const char *_test1 = "/tmp/usqlite_test1.db";
static const char *_test2 = "/tmp/usqlite_test2.db";

int usqlite_test_run(int argc, const char **argv) {
    testing::InitGoogleTest(&argc, const_cast<char **>(argv));
    return RUN_ALL_TESTS();
}

#pragma mark - global test
TEST(usqlite_tests, open_close_database)
{
    DBConnection connection(_db);
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

TEST(usqlite_tests, attach_detach_database)
{
    DBConnection con(_test1);
    con.open();
    
    const std::string schema = "test2";
    EXPECT_TRUE(con.attachDatabase(_test2, schema));
    
    std::vector<std::string> test_tables;
    test_tables.push_back("test_table1");
    test_tables.push_back("test_table2");
    test_tables.push_back("test_table3");
    for (auto i = test_tables.begin(); i != test_tables.end(); ++i) {
        auto create = TableCommand::create(*i);
        create.schema("test2")
        .columnDef("a", "int");
        
        EXPECT_TRUE(con.exec(create.command()));
    }
    
    EXPECT_EQ(0, con.allTables("main").size());
    EXPECT_EQ(test_tables, con.allTables(schema));
    
    auto dbs = con.allDatabase();
    EXPECT_EQ(2, dbs.size());
    EXPECT_EQ("main", dbs[0].name);
    EXPECT_EQ(_test1, dbs[0].filepath);
    
    EXPECT_EQ(schema, dbs[1].name);
    EXPECT_EQ(_test2, dbs[1].filepath);
    
    con.detachDatabase(schema);
    EXPECT_EQ(0, con.allTables(schema).size());
    
    con.close();
    
    std::remove(_test1);
    std::remove(_test2);
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
        
        std::remove(_db);
    }
    
    bool createTable() {
        return _connection.exec("create table if not exists use_sqlite_table(a text, b int, c real, d blob, e boolean)");
    }
    
    bool dropTable() {
        auto tables = _connection.allTables();
        for (auto iter = tables.begin(); iter != tables.end(); ++iter) {
            _connection.exec(TableCommand::drop(*iter).command());
        }
        
        return true;
    }
    
    bool insertRow(const std::string &a, int b, double c, bool e = false) {
        std::stringstream ss;
        ss<<"insert into use_sqlite_table (a, b, c, e) values ('"<<a<<"', "<<b<<", "<<c<<", "<<(e ? 1 : 0)<<")";
        std::string cmd = ss.str();
        return _connection.exec(cmd);
    }
    
    bool clearTable() {
        return _connection.exec("delete from use_sqlite_table");
    }
    
protected:
    DBConnection _connection;
};

TEST_F(USQLTests, connection_tables)
{
    dropTable();
    EXPECT_EQ(0, _connection.allTables().size());
    
    std::vector<std::string> tables;
    tables.push_back("test1");
    tables.push_back("test2");
    tables.push_back("test3");
    for (auto iter = tables.begin(); iter != tables.end(); ++iter) {
        auto cmd = TableCommand::create(*iter);
        cmd.columnDef("a", "int");
        EXPECT_TRUE(_connection.exec(cmd.command()));
    }
    
    for (auto iter = tables.begin(); iter != tables.end(); ++iter) {
        EXPECT_TRUE(_connection.tableExists(*iter));
    }
    auto ret = _connection.allTables();
    EXPECT_EQ(tables, ret);

}

TEST_F(USQLTests, connection_table_info)
{
    const std::string tablename = "test_table";
    std::vector<std::string> primaryKeys;
    primaryKeys.push_back("a");
    primaryKeys.push_back("c");
    auto cmd = TableCommand::create(tablename);
    cmd.createIfNotExist(false)
    .columnDef("a", "int", "default 11")
    .columnDef("b", "text", "not null default 'hello world'")
    .columnDef("c", "real", "default 12.3")
    .tableConstraintPrimaryKey(primaryKeys);
    
    EXPECT_TRUE(_connection.exec(cmd.command()));
    auto table = _connection.tableInfo(tablename);
    EXPECT_EQ(tablename, table.name);
    EXPECT_EQ(3, table.columndefs.size());
    
    auto a = std::find_if(table.columndefs.begin(), table.columndefs.end(), [](const DBConnection::ColumnInfo &c){
        return c.name == "a";
    });
    EXPECT_TRUE(a != table.columndefs.end());
    EXPECT_EQ("a", a->name);
    EXPECT_EQ("int", a->type);
    EXPECT_TRUE(a->nullable);
    EXPECT_EQ("11", a->defaultValue);
    
    auto b = std::find_if(table.columndefs.begin(), table.columndefs.end(), [](const DBConnection::ColumnInfo &c){
        return c.name == "b";
    });
    EXPECT_TRUE(a != table.columndefs.end());
    EXPECT_EQ("b", b->name);
    EXPECT_EQ("text", b->type);
    EXPECT_FALSE(b->nullable);
    EXPECT_EQ("'hello world'", b->defaultValue);
    
    auto c = std::find_if(table.columndefs.begin(), table.columndefs.end(), [](const DBConnection::ColumnInfo &c){
        return c.name == "c";
    });
    EXPECT_TRUE(a != table.columndefs.end());
    EXPECT_EQ("c", c->name);
    EXPECT_EQ("real", c->type);
    EXPECT_TRUE(c->nullable);
    EXPECT_EQ("12.3", c->defaultValue);
    
    EXPECT_EQ(2, table.primaryKeys.size());
    EXPECT_TRUE(std::find(table.primaryKeys.begin(), table.primaryKeys.end(), "a") != table.primaryKeys.end());
    EXPECT_TRUE(std::find(table.primaryKeys.begin(), table.primaryKeys.end(), "b") == table.primaryKeys.end());
    EXPECT_TRUE(std::find(table.primaryKeys.begin(), table.primaryKeys.end(), "c") != table.primaryKeys.end());
}

TEST_F(USQLTests, fail_on_closed_database)
{
    EXPECT_TRUE(insertRow("row 1", 10, 12.3));
    
    std::string cmd("select * from use_sqlite_table");
    EXPECT_TRUE(_connection.exec(cmd));
    
    Query query(cmd, _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(5, query.columnCount());
    EXPECT_EQ("row 1", query.textForName("a"));
    EXPECT_EQ(10, query.intForName("b"));
    
    EXPECT_TRUE(_connection.close());
    EXPECT_FALSE(_connection.exec(cmd));
    Query failed(cmd, _connection);
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
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_TRUE(query.reset());
    
    EXPECT_TRUE(_connection.close());
    EXPECT_FALSE(query.next());
    EXPECT_FALSE(query.reset());
}

TEST_F(USQLTests, query_reset)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_FALSE(query.next());
    
    EXPECT_TRUE(query.reset());
    EXPECT_TRUE(query.next());
}

TEST_F(USQLTests, query_column)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    
    EXPECT_EQ(5, query.columnCount());
    EXPECT_EQ(0, query.columnIndexForName("a"));
    EXPECT_EQ(USQL_INVALID_COLUMN_INDEX, query.columnIndexForName("A"));
    EXPECT_EQ(1, query.columnIndexForName("b"));
    EXPECT_EQ(2, query.columnIndexForName("c"));
    
    EXPECT_TRUE(query.availableIndex(0));
    EXPECT_TRUE(query.availableIndex(3));
    EXPECT_FALSE(query.availableIndex(-1));
    EXPECT_FALSE(query.availableIndex(5));
}

TEST_F(USQLTests, query_column_type)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    Query query("select * from use_sqlite_table", _connection);
    
    EXPECT_TRUE(query.next());
    
    EXPECT_EQ(ColumnType::Text, query.typeForName("a"));
    EXPECT_EQ(ColumnType::Integer, query.typeForName("b"));
    EXPECT_EQ(ColumnType::Float, query.typeForName("c"));
    EXPECT_EQ(ColumnType::Null, query.typeForName("d"));
    EXPECT_EQ(ColumnType::Integer, query.typeForName("e"));
    EXPECT_EQ(ColumnType::InvalidType, query.typeForName("A"));
}

TEST_F(USQLTests, query_int)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
    EXPECT_EQ(10, query.int64ForName("b"));
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("a"));
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_boolean)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3, true));
    
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(ColumnType::Integer, query.typeForName("e"));
    EXPECT_EQ(true, query.booleanForName("e"));
    EXPECT_EQ(true, query.booleanForColumnIndex(4));
}

TEST_F(USQLTests, query_text)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(USQL_ERROR_TEXT, query.textForName("b"));
    EXPECT_EQ("hello world", query.textForName("a"));
    EXPECT_EQ(USQL_ERROR_TEXT, query.textForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_double)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(USQL_ERROR_FLOAT, query.floatForName("b"));
    EXPECT_EQ(USQL_ERROR_FLOAT, query.floatForName("a"));
    EXPECT_EQ(12.3, query.floatForName("c"));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLTests, query_close)
{
    EXPECT_TRUE(insertRow("hello world", 10, 12.3));
    Query query("select * from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
    
    query.close();
    EXPECT_FALSE(query.next());
    EXPECT_EQ(USQL_ERROR_INTEGER, query.intForName("b"));
    
    query.reset();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("b"));
}

TEST_F(USQLTests, cursor_bind)
{
    Cursor stmt("insert into use_sqlite_table (a, b, c, d) values (:a, :b, :c, :d)", _connection);
    const std::string tvalue = "bind hello world";
    int ivalue = 123;
    double rvalue = 111.23;
    const std::string bvalue = "binary data";
    EXPECT_TRUE(stmt.bind(":a", tvalue));
    EXPECT_TRUE(stmt.bind(":b", ivalue));
    EXPECT_TRUE(stmt.bind(":c", rvalue));
    EXPECT_TRUE(stmt.bind(":d", bvalue.data(), (int)bvalue.size()));
    EXPECT_TRUE(stmt.exec());
    
    Cursor query("select * from use_sqlite_table where a = ? and b = ? and c = ? and d = ?", _connection);
    EXPECT_TRUE(query.bind(1, tvalue));
    EXPECT_TRUE(query.bind(2, ivalue));
    EXPECT_TRUE(query.bind(3, rvalue));
    EXPECT_TRUE(query.bind(4, bvalue.data(), (int)bvalue.size()));
    EXPECT_TRUE(query.next());
}

TEST_F(USQLTests, connnection_transaction)
{
    Query query("select count(*) as row_count from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(0, query.intForColumnIndex(0));
    query.close();
    
    _connection.transaction(TransactionType::Deferred
                            , [this]()->bool{
                                this->insertRow("hello world", 10, 10.12);
                                this->insertRow("row 2", 110, 12.22);
                                return false;
                            });
    query.reset();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(0, query.intForColumnIndex(0));
    query.close();
    
    _connection.transaction(TransactionType::Deferred
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

TEST_F(USQLTests, connection_function)
{
    auto func = [](sqlite3_context* context, std::vector<sqlite3_value *> &argv){
        if (argv.size() >= 1) {
            auto text = sqlite3_value_text(argv[0]);
            if (text) {
                int len = (int)std::strlen((const char *)text);
                sqlite3_result_int(context, len);
                return ;
            }
        }
        
        sqlite3_result_int(context, -1);
    };
    
    Function *obj = Function::create("usql_string_len");
    obj->setFunction(func);
    obj->setArgumentCount(1);
    obj->deterministic = true;
    EXPECT_TRUE(_connection.registerFunction(obj));
    
    insertRow("hello world", 10, 12.3);
    Query query("select usql_string_len(a) as string_len from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(11, query.intForName("string_len"));
    
    _connection.close();
}

TEST_F(USQLTests, connection_aggregate_function)
{
    static int maxLen = -1;
    AggregateFunction *obj = AggregateFunction::create("max_text_len");
    obj->setFunction([&](sqlite3_context* context, std::vector<sqlite3_value *> &argv){
        if (argv.size() >= 1) {
            auto text = sqlite3_value_text(argv[0]);
            if (text) {
                maxLen = std::max(maxLen, (int)std::strlen((const char *)text));
            }
        }
    });
    obj->setFinalFunction([](sqlite3_context *context){
        sqlite3_result_int(context, maxLen);
    });
    EXPECT_TRUE(_connection.registerFunction(obj));
    
    insertRow("hello world", 10, 12.3);
    insertRow("bla bla bla bla bla...", 11, 34.1);
    Query query("select max_text_len(a) as max_len from use_sqlite_table", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(22, query.intForName("max_len"));
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
        auto tables = _connection.allTables();
        for (auto iter = tables.begin(); iter != tables.end(); ++iter) {
            _connection.exec(TableCommand::drop(*iter).ifExists(true).command());
        }
        _connection.close();
        std::remove(_db);
    }
    
    DBConnection _connection;
    std::string _testTablename = "test_table_name";
};

TEST_F(USQLExtTests, tablename_check)
{
    EXPECT_TRUE(TableCommand::checkTablename("test_table_name"));
    EXPECT_FALSE(TableCommand::checkTablename("sqlite_test_table_name"));
}

TEST_F(USQLExtTests, create_table)
{
    std::map<ColumnConstraint, std::string> opta;
    opta[ColumnConstraint::PrimaryKey] = "";
    
    std::map<ColumnConstraint, std::string> optb;
    optb[ColumnConstraint::NotNull] = "";
    optb[ColumnConstraint::Unique] = "";
    optb[ColumnConstraint::Check] = "(b > 100)";
    
    std::map<ColumnConstraint, std::string> optc;
    optc[ColumnConstraint::NotNull] = "";
    optc[ColumnConstraint::Default] = "'hello world'";
    optc[ColumnConstraint::Collate] = "NOCASE";
    
    std::vector<std::string> unique;
    unique.push_back("a");
    unique.push_back("c");
    
    auto cmd = TableCommand::create(_testTablename);
    cmd.temp(false).createIfNotExist(true)
    .columnDef("a", "int", opta)
    .columnDef("b", "int", optb)
    .columnDef("c", "text", optc)
    .tableConstraintCheck("(a + b < 200)")
    .tableConstraintUnique(unique)
    .withoutRowId(false);
    
    EXPECT_TRUE(_connection.exec(cmd.command()));
    EXPECT_TRUE(_connection.tableExists(_testTablename));
    EXPECT_TRUE(_connection.exec("insert into test_table_name (a, b) values (1, 101)"));
    EXPECT_FALSE(_connection.exec("insert into test_table_name (a, b) values (2, 10)"));
    EXPECT_FALSE(_connection.exec("insert into test_table_name (a, b) values (3, 197)"));
    
    Query query("select * from test_table_name where c = 'HELLO WORLD'", _connection);
    EXPECT_TRUE(query.next());
}

TEST_F(USQLExtTests, drop_table)
{
    auto cmd = TableCommand::create(_testTablename);
    cmd.createIfNotExist(true)
    .columnDef("a", "int", "primary key");
    
    EXPECT_TRUE(_connection.exec(cmd.command()));
    EXPECT_TRUE(_connection.tableExists(_testTablename));
    
    auto drop = TableCommand::drop(_testTablename);
    drop.ifExists(true);
    EXPECT_TRUE(_connection.exec(drop.command()));
    EXPECT_FALSE(_connection.tableExists(_testTablename));
}

TEST_F(USQLExtTests, rename_table)
{
    auto create = TableCommand::create(_testTablename);
    create.createIfNotExist(true)
    .columnDef("a", "int");
    
    EXPECT_TRUE(_connection.exec(create.command()));
    EXPECT_TRUE(_connection.tableExists(_testTablename));
    
    const std::string newname = "new_test_table";
    EXPECT_TRUE(_connection.exec(TableCommand::rename(_testTablename, newname)));
    EXPECT_FALSE(_connection.tableExists(_testTablename));
    EXPECT_TRUE(_connection.tableExists(newname));
    
    EXPECT_TRUE(_connection.exec(TableCommand::drop(newname).command()));
}

TEST_F(USQLExtTests, alter_table)
{
    auto create = TableCommand::create(_testTablename);
    create.createIfNotExist(true)
    .columnDef("a", "int");
    EXPECT_TRUE(_connection.exec(create.command()));
    EXPECT_TRUE(_connection.exec("insert into test_table_name (a) values (1)"));
    
    std::string select = "select * from " + _testTablename;
    Query query(select, _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(0, query.columnIndexForName("a"));
    EXPECT_TRUE(query.columnIndexForName("b") < 0);
    
    auto cmd = TableCommand::alter(_testTablename);
    EXPECT_EQ("", cmd.command());
    cmd.columnDef("b", "text");
    EXPECT_TRUE(_connection.exec(cmd.command()));
    
    query.reset();
    EXPECT_TRUE(query.next());
    EXPECT_EQ(0, query.columnIndexForName("a"));
    EXPECT_EQ(1, query.columnIndexForName("b"));
}

TEST_F(USQLExtTests, insert_row)
{
    auto create = TableCommand::create(_testTablename);
    create.createIfNotExist(true)
    .columnDef("a", "int")
    .columnDef("b", "text")
    .columnDef("c", "real")
    .columnDef("d", "datetime")
    .columnDef("e", "datetime")
    .columnDef("f", "datetime");
    EXPECT_TRUE(_connection.exec(create.command()));
    
    auto ts = std::time(nullptr);
    
    auto cmd = InsertCommand(_testTablename);
    cmd.value("a", 10)
    .value("b", "hello world")
    .value("c", 12.3)
    .datetimeNow("d")
    .expr("e", "strftime('%s', 'now')")
    .expr("f", "julianday('now')");
    
    EXPECT_TRUE(_connection.exec(cmd.command()));
//    EXPECT_EQ("", _connection.lastErrorMessage());
    
    Query query("select * from test_table_name", _connection);
    EXPECT_TRUE(query.next());
    EXPECT_EQ(10, query.intForName("a"));
    EXPECT_EQ("hello world", query.textForName("b"));
    EXPECT_EQ(12.3, query.floatForName("c"));
    EXPECT_TRUE(std::difftime(query.datetimeForName("d"), ts) < 1);
    EXPECT_TRUE(std::difftime(query.datetimeForName("e"), ts) < 1);
    EXPECT_TRUE(std::difftime(query.datetimeForName("f"), ts) < 1);
}

TEST_F(USQLExtTests, delete_row)
{
    auto create = TableCommand::create(_testTablename);
    create.createIfNotExist(true)
    .columnDef("a", "int")
    .columnDef("b", "text")
    .columnDef("c", "real");
    EXPECT_TRUE(_connection.exec(create.command()));
    
    auto cmd = InsertCommand(_testTablename);
    cmd.value("a", 10)
    .value("b", "hello world")
    .value("c", 12.3);
    EXPECT_TRUE(_connection.exec(cmd.command()));
    
    EXPECT_TRUE(_connection.exec(create.command()));
    Query query("select * from test_table_name where a = 10", _connection);
    EXPECT_TRUE(query.next());
    query.reset();
    
    auto deletecmd = DeleteCommand(_testTablename);
    deletecmd.where("a = 20");
    EXPECT_TRUE(_connection.exec(deletecmd.command()));
    EXPECT_TRUE(query.next());
    query.reset();
    
    deletecmd = DeleteCommand(_testTablename);
    deletecmd.where("a = 10");
    EXPECT_TRUE(_connection.exec(deletecmd.command()));
    EXPECT_FALSE(query.next());
}

TEST_F(USQLExtTests, update_row)
{
    auto create = TableCommand::create(_testTablename);
    create.createIfNotExist(true)
    .columnDef("a", "int")
    .columnDef("b", "text")
    .columnDef("c", "real");
    EXPECT_TRUE(_connection.exec(create.command()));
    
    auto cmd = InsertCommand(_testTablename);
    cmd.value("a", 10)
    .value("b", "hello world")
    .value("c", 12.3);
    EXPECT_TRUE(_connection.exec(cmd.command()));
    
    EXPECT_TRUE(_connection.exec(create.command()));
    Query query("select * from test_table_name where a = 10", _connection);
    EXPECT_TRUE(query.next());
    query.reset();
    
    auto update = UpdateCommand(_testTablename);
    update.value("a", 20)
    .where("a = 10");
    EXPECT_TRUE(_connection.exec(update.command()));
    
    EXPECT_FALSE(query.next());
    
    Cursor cursor("select * from test_table_name where a=:a", _connection);
    cursor.bind(":a", 20);
    EXPECT_TRUE(cursor.next());
}