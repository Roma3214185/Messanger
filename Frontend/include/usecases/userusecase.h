#ifndef USERUSECASE_H
#define USERUSECASE_H

#include <QObject>

#include "managers/usermanager.h"

class IUserDataManager;
class TokenManager;
struct User;

class UserUseCase : public QObject {
  Q_OBJECT
 public:
  UserUseCase(IUserDataManager *, std::unique_ptr<UserManager>, TokenManager *);
  void getUserAsync(long long user_id);
  std::optional<User> getUser(long long user_id);
  QList<User> findUsers(const QString &text);

 private:
  std::unique_ptr<UserManager> user_manager_;
  IUserDataManager *data_manager_;
  TokenManager *token_manager_;
};

#endif  // USERUSECASE_H
