#include "usecases/userusecase.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include "dto/User.h"
#include "managers/TokenManager.h"
#include "managers/datamanager.h"
#include "managers/usermanager.h"

namespace {

template <typename T>
T waitForFuture(QFuture<T> &future) {
  QFutureWatcher<T> watcher;
  watcher.setFuture(future);

  QEventLoop loop;
  QObject::connect(&watcher, &QFutureWatcher<T>::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return future.result();
}

}  // namespace

UserUseCase::UserUseCase(IUserDataManager *data_manager, std::unique_ptr<UserManager> user_manager,
                         TokenManager *token_manager)
    : user_manager_(std::move(user_manager)), data_manager_(data_manager), token_manager_(token_manager) {}

auto UserUseCase::getUser(long long user_id) -> std::optional<User> {
  auto future = user_manager_->getUser(user_id, token_manager_->getToken());
  return waitForFuture(future);
}

void UserUseCase::getUserAsync(long long user_id) {
  if (auto user = data_manager_->getUser(user_id); user.has_value()) return;
  auto future = user_manager_->getUser(user_id, token_manager_->getToken());

  future
      .then(this,
            [this, user_id](std::optional<User> userOpt) {
              if (!userOpt) {
                LOG_ERROR("Can't get info about user {}", user_id);
              } else {
                data_manager_->save(*userOpt);
              }
            })
      .onFailed(this, [user_id]() { LOG_ERROR("Error in getUserAsync for user_id {}", user_id); });
}

QList<User> UserUseCase::findUsers(const QString &text) {
  auto future = user_manager_->findUsersByTag(text, token_manager_->getToken());
  return waitForFuture(future);
}
