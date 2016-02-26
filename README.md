UseSQL
==========
sqlite c++ wrapper

### Requirement
	Xcode 7.2+
    
Example:
==========
### Open Database
    DBConnection connection("/tmp/usqlite.db");
    db.open();
    db.exec("create table if not exists table_name(a int, b real, c text)");
    connection.exec("insert into table_name (a, b, c) values (10, 11.2, 'hello world')");
    db.close();
    
### Query
    Query query("select * from table_name", db);
    while(query.next()) {
      query.intForName("a");
      query.floatForName("b");
      query.textForName("c");
    }
    
### Bind
    Cursor cursor("insert into table_name (a, b, c) values (:a, :b, :c)", _connection);
    cursor.bind(":a", 10);
    cursor.bind(":b", 11.2);
    cursor.bind(":c", "hello world");
    cursor.exec();

### See Also
[sqlite doc](http://www.sqlite.org)
