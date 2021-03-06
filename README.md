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
    Connection db("/tmp/example.db");
    db.open();
    db.exec("create table if not exists table_name(a int, b real, c text)");
    db.exec("insert into table_name (a, b, c) values (10, 11.2, 'hello world')");

### Transaction
    db.transaction(TransactionType::Deferred, [](Connection &con)->bool{
        con.exec("insert into table_name (a, b, c) values (1, 1.1, 'step one')");
        con.exec("insert into table_name (a, b, c) values (2, 2.2, 'step two')");
        return true;
    });
    
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

### Create Function
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

### Create Aggregate Function
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

### See Also
[sqlite doc](http://www.sqlite.org)
