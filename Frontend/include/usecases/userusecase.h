#ifndef USERUSECASE_H
#define USERUSECASE_H

#include <QObject>

class UserManager;
class IUserDataManager;
class TokenManager;
struct User;

class IUserUseCase {
 public:
  using Token = QString;
  virtual ~IUserUseCase() = default;
  virtual void getUserAsync(long long user_id) = 0;
  virtual std::optional<User> getUser(long long user_id) = 0;
  virtual QList<User> findUsers(const QString &text) = 0;
};

class UserUseCase : public QObject, public IUserUseCase {
  Q_OBJECT
 public:
  UserUseCase(IUserDataManager *, std::unique_ptr<UserManager>, TokenManager *);
  void getUserAsync(long long user_id) override;
  std::optional<User> getUser(long long user_id) override;
  QList<User> findUsers(const QString &text) override;

 private:
  std::unique_ptr<UserManager> user_manager_;
  IUserDataManager *data_manager_;
  TokenManager *token_manager_;
};

#endif  // USERUSECASE_H
