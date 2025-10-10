#include "authmanager.h"
#include "Headers/JwtUtils.h"
#include <QDebug>

AuthManager::AuthManager(UserRepository& repository)
    : rep(repository) { }

std::optional<AuthResponce> AuthManager::getUser(std::string token){
    qDebug() << "[INFO] Get user from token: " << token;
    auto id = JwtUtils::verifyTokenAndGetUserId(token);
    if(!id) {
        qDebug() << "[ERROR] Server can't verify token: ";
        return std::nullopt;
    }

    qDebug() << "[INFO] Server verified token; id = " << *id;

    auto findedUser = rep.findById(*id);
    if(!findedUser) {
        qDebug() << "[ERROR] User with id not founded; id = " << *id;
        return std::nullopt;
    }

    qDebug() << "[INFO] User was founded; id = " << findedUser->name;

    AuthResponce res{
        .token = token,
        .user = findedUser
    };

    return res;
}

std::optional<AuthResponce> AuthManager::loginUser(std::string email, std::string password){
    auto findedUser = rep.findByEmail(email);
    if(!findedUser) return std::nullopt;

    auto token = JwtUtils::generateToken(findedUser->id);
    //cash.createToken(token, user->id);
    AuthResponce res{
        .token = token,
        .user = findedUser
    };
    return res;
}

std::optional<AuthResponce> AuthManager::registerUser(RegisterRequest req){
    auto createdUser = rep.createUser(req);
    if(!createdUser) return std::nullopt;

    auto token = JwtUtils::generateToken(createdUser->id);

    //cash.createToken(token, user->id);
    AuthResponce res{
        .token = token,
        .user = createdUser
    };

    return res;
}

QList<User> AuthManager::findUserByTag(std::string tag){
    auto findedUsers = rep.findByTag(tag);
    return findedUsers;
}

std::optional<User> AuthManager::findUserById(int userId){
    return rep.findById(userId);
}
