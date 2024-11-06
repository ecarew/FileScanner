extern "C"{
#include <sqlite3.h>
}
#include <string>
#include <iostream>

void initdb(sqlite3 *db){
    std::string sql(R"END(
    PRAGMA foreign_keys=OFF;
    BEGIN TRANSACTION;
    CREATE TABLE directory(
    id integer primary key autoincrement,
    parent integer,
    last_altered TEXT,
    name TEXT);
    INSERT INTO directory VALUES(1,1,'','/Users/evancarew/Projects');
    CREATE TABLE files(
    name TEXT,
    dir integer,
    last_updated TEXT,
    FOREIGN KEY(dir) REFERENCES directory(id));
    CREATE TABLE operations(
    op text,
    duration REAL);
    INSERT INTO operations VALUES('archive',180.0);
    INSERT INTO operations VALUES('remove',720.0);
    DELETE FROM sqlite_sequence;
    INSERT INTO sqlite_sequence VALUES('directory',1);
    COMMIT;
    )END" );
    char *zErrMsg = 0;
    int rc = 0;
    rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &zErrMsg);
    if(rc!=SQLITE_OK){
        std::cout << "SQL Error: " << zErrMsg << "\n";
        sqlite3_free(zErrMsg);
    }

}
