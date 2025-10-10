#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>

class DataBase
{
private:
    sqlite3* db;
public:
    DataBase(const std::string& filename);
    ~DataBase() { sqlite3_close(db); }
    sqlite3* getHandle() { return db; }
};

#endif // DATABASE_H
