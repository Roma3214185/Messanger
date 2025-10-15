#ifndef DATABASE_H
#define DATABASE_H

#include <QList>

#include "DataBase/database.h"
#include "Headers/RegisterRequest.h"
#include "Headers/User.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/qsqlquery.h>
#include "Headers/User.h"

class User;

using OptionalUser = std::optional<User>;

class DataBase
{

public:

    DataBase();

    OptionalUser findById(int id);
    OptionalUser findByEmail(std::string email);
    OptionalUser createUser(RegisterRequest req);
    QList<User> findByTag(std::string tag);
    void clear();

private:

    void createUserDataBase();
    QSqlDatabase getThreadDatabase();
    User getUserFromQuery(const QSqlQuery& query);

    template<typename... Args>
    bool executeQuery(QSqlQuery& query, Args&&... args);
};


#endif // DATABASE_H
