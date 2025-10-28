#include "authmanager.h"
#include <optional>
#include <JwtUtils.h>
#include "Debug_profiling.h"

using std::string;
using std::nullopt;

OptionalResponce AuthManager::getUser(const string& token){
    PROFILE_SCOPE("[AuthManager::getUser");
    auto id = JwtUtils::verifyTokenAndGetUserId(token);
    if(!id) {
        spdlog::error("[getUser] Server can't verify token (NULLPTR)");
        return nullopt;
    }

    if(*id == 0) {
        spdlog::error("[getUser] Server can't verify token (id is zero)");
        return nullopt;
    }

    LOG_INFO("[getUser] verified id = '{}'", *id);

    auto findedUser = rep.findOne<User>(*id);
    if(!findedUser) {
        spdlog::error("User with id not founded; id = '{}'", *id);
        return nullopt;
    }

    LOG_INFO("User was founded; name = '{}'", findedUser->username);

    return AuthResponce{
        .token = token,
        .user = findedUser
    };
}

OptionalResponce AuthManager::loginUser(const string& email, const string& password){
    PROFILE_SCOPE("[AuthManager::loginUser");
    spdlog::debug("Try login user, email = '{}' and 'password '{}'", email, password);
    auto findedUsers = rep.findByField<User>("email", QString::fromStdString(email));

    //find hashedPassword and check
    if(findedUsers.empty()) {
        spdlog::warn("User not found with email '{}'", email);
        return nullopt;
    }

    auto findedUser = findedUsers.front();
    LOG_INFO("User found with email '{}', id is '{}'", email, findedUser.id);

    auto token = JwtUtils::generateToken(findedUser.id);
    return AuthResponce{
        .token = token,
        .user = findedUser
    };
}

OptionalResponce AuthManager::registerUser(const RegisterRequest& req){
    PROFILE_SCOPE("AuthManager::registerUser");
    User userToSave{
        .username = req.name,
        .tag = req.tag,
        .email = req.email    //u don't save password now
    };

    rep.save(userToSave);
    auto token = JwtUtils::generateToken(userToSave.id);

    return AuthResponce {
        .token = token,
        .user = userToSave
    };
}

std::vector<User> AuthManager::findUserByTag(const string& tag){
    PROFILE_SCOPE("AuthManager::findUserByTag");
    auto findedUsers = rep.findByField<User>("tag", QString::fromStdString(tag));
    // TODO: make "tag" -> User::UserTag
    return findedUsers;
}

OptionalUser AuthManager::findUserById(const int& userId){
    PROFILE_SCOPE("AuthManager::findUserById");
    return rep.findOne<User>(userId);
}
