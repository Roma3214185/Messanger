#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "DataBase/database.h"
#include "Headers/RegisterRequest.h"
#include "Headers/User.h"
#include <QList>

class UserRepository
{
    DataBase database_;
public:
    UserRepository(DataBase& database);

    std::optional<User> findById(int id);
    std::optional<User> findByEmail(std::string email);
    std::optional<User> createUser(RegisterRequest req);
    QList<User> findByTag(std::string tag);
    void clear();
private:
    void createUserDataBase();
};

#endif // USERREPOSITORY_H
