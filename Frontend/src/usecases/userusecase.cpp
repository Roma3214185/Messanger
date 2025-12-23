#include "usecases/userusecase.h"

#include <QFuture>
#include <QFutureWatcher>

#include "managers/usermanager.h"
#include "managers/datamanager.h"
#include "managers/TokenManager.h"
#include "dto/User.h"

namespace {

template <typename T>
T waitForFuture(QFuture<T>& future) {
  QFutureWatcher<T> watcher;
  watcher.setFuture(future);

  QEventLoop loop;
  QObject::connect(&watcher, &QFutureWatcher<T>::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return future.result();
}

}  // namespace

UserUseCase::UserUseCase(DataManager* data_manager, UserManager* user_manager, TokenManager* token_manager)
    : user_manager_(user_manager), data_manager_(data_manager), token_manager_(token_manager) {}

auto UserUseCase::getUser(long long user_id) -> std::optional<User> {
  auto future = user_manager_->getUser(user_id, token_manager_->getToken());
  return waitForFuture(future);
}

void UserUseCase::getUserAsync(long long user_id) {
  if(data_manager_->getUser(user_id)) return;

  QFuture<std::optional<User>> future = user_manager_->getUser(user_id, token_manager_->getToken());

  auto *watcher = new QFutureWatcher<std::optional<User>>(this);
  QObject::connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher, user_id]() {
    auto userOpt = watcher->result();
    watcher->deleteLater();

    if (!userOpt) {
      LOG_ERROR("Can't get info about user {}", user_id);
      return;
    }

    data_manager_->saveUser(*userOpt);
  });

  watcher->setFuture(future);
}

QList<User> UserUseCase::findUsers(const QString& text) {
  auto future = user_manager_->findUsersByTag(text, token_manager_->getToken());
  return waitForFuture(future);
}
