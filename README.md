UseSQL
==========
sqlite c++ wrapper

### Support Platform
    IOS
    OS X
    

### Requirement
	Xcode 7.2+
    
Useage:
==========
### Open Database
    DBConnection connection("/tmp/usqlite.db");
    db.open();
    db.exec("create table if not exists table_name(a int, b real, c text)");
    db.exec("insert into table_name (a, b, c) values (10, 11.2, 'hello world')");
    db.close();
    
### Query
    Query query("select * from table_name", db);
    while(query.next()) {
      query.intForName("a");
      query.floatForName("b");
      query.textForName("c");
    }
    
### Bind
    Cursor cursor("insert into table_name (a, b, c) values (:a, :b, :c)", db);
    cursor.bind(":a", 10);
    cursor.bind(":b", 11.2);
    cursor.bind(":c", "hello world");
    cursor.exec();

### Command
    auto create = TableCommand::create("table_name");
    create.createIfNotExist(true)
    .columnDef("a", "int", "default 11")
    .columnDef("b", "real", "default 12.3")
    .columnDef("c", "text", "not null default 'hello world'");

    db.exec(create.command());

==========
### Todo
    Custom function
    Online backup
    Custom VFS
    Config

### See Also
[sqlite doc](http://www.sqlite.org)
