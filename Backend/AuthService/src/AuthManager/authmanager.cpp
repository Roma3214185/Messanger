#include "authmanager.h"
#include "Headers/JwtUtils.h"
#include <QDebug>
#include <optional>

using std::string;
using std::nullopt;

AuthManager::AuthManager(DataBase& database)
    : db(database)
{

}

OptionalResponce AuthManager::getUser(const string& token){
    auto id = JwtUtils::verifyTokenAndGetUserId(token);
    if(!id) {
        qDebug() << "[ERROR] Server can't verify token: ";
        return nullopt;
    }

    auto findedUser = db.findById(*id);
    if(!findedUser) {
        qDebug() << "[ERROR] User with id not founded; id = " << *id;
        return nullopt;
    }

    qDebug() << "[INFO] User was founded; id = " << findedUser->username;

    return AuthResponce{
        .token = token,
        .user = findedUser
    };
}

OptionalResponce AuthManager::loginUser(const string& email, const string& password){
    auto findedUser = db.findByEmail(email);
    if(!findedUser) return nullopt;

    auto token = JwtUtils::generateToken(findedUser->id);

    return AuthResponce{
        .token = token,
        .user = findedUser
    };
}

OptionalResponce AuthManager::registerUser(const RegisterRequest& req){
    auto createdUser = db.createUser(req);
    if(!createdUser) return nullopt;

    auto token = JwtUtils::generateToken(createdUser->id);

    return AuthResponce {
        .token = token,
        .user = createdUser
    };
}

QList<User> AuthManager::findUserByTag(const string& tag){
    auto findedUsers = db.findByTag(tag);
    return findedUsers;
}

OptionalUser AuthManager::findUserById(const int& userId){
    return db.findById(userId);
}
