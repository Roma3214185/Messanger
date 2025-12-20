#ifndef MOCKAUTHMANAGER_H
#define MOCKAUTHMANAGER_H

#include "authservice/interfaces/IAuthManager.h"

class MockAuthManager : public IAuthManager {
  public:
    int call_getUser = 0;
    int call_loginUser = 0;
    int call_registerUser = 0;
    int call_findUserByTag = 0;

    int last_user_id;
    std::string last_tag;
    std::optional<User> mock_user;
    LoginRequest last_login_request;
    RegisterRequest last_register_request;
    std::vector<User> mock_users;

    OptionalUser  getUser(long long user_id) override {
      ++call_getUser;
      last_user_id = user_id;
      return mock_user;
    }

    OptionalUser loginUser(const LoginRequest& login_request) override {
      ++call_loginUser;
      last_login_request = login_request;
      return mock_user;
    }

    OptionalUser registerUser(const RegisterRequest& req) override {
      ++call_registerUser;
      last_register_request = req;
      return mock_user;
    }

    std::vector<User> findUsersByTag(const std::string& tag) override {
      ++call_findUserByTag;
      last_tag = tag;
      return mock_users;
    }
};


#endif // MOCKAUTHMANAGER_H
