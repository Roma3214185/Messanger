#include "database.h"
#include <string>

DataBase::DataBase(const std::string& filename){ sqlite3_open(filename.c_str(), &db); }
