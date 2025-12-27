#ifndef USERUSECASE_H
#define USERUSECASE_H

#include <QObject>

class UserManager;
class DataManager;
class TokenManager;
struct User;

class UserUseCase : public QObject {
    Q_OBJECT
  public:
    using Token = QString;
    UserUseCase(DataManager*, UserManager*, TokenManager*);
    void getUserAsync(long long user_id);
    std::optional<User> getUser(long long user_id);
    QList<User> findUsers(const QString& text);

  private:
    UserManager* user_manager_;
    DataManager* data_manager_;
    TokenManager* token_manager_;
};

#endif // USERUSECASE_H
