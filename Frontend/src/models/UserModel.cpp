#include "models/UserModel.h"

#include "DebugProfiling/Debug_profiling.h"

UserModel::UserModel(QObject* parent) : QAbstractListModel(parent) {}

int UserModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return users_.size();
}

void UserModel::addUser(const User& user) {
  beginInsertRows(QModelIndex(), users_.size(), users_.size());
  LOG_INFO("User model add user ({}) with email: {}", user.name.toStdString(), user.email.toStdString());
  users_.push_back(user);
  endInsertRows();
}

void UserModel::clear() {
  beginResetModel();
  users_.clear();
  endResetModel();
}

QVariant UserModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= users_.size()) return QVariant();

  const User& user = users_[index.row()];
  switch (role) {
    case UserModel::Roles::UserIdRole:
      return user.id;
    case UserModel::Roles::NameRole:
      return user.name;
    case UserModel::Roles::TagRole:
      return user.tag;
    case UserModel::Roles::EmailTimeRole:
      return user.email;
    case UserModel::Roles::AvatarRole:
      return user.avatarPath;
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> UserModel::roleNames() const {
  return {{UserModel::Roles::UserIdRole, "chat_id"},
          {UserModel::Roles::NameRole, "name"},
          {UserModel::Roles::TagRole, "tag"},
          {UserModel::Roles::EmailTimeRole, "email"},
          {UserModel::Roles::AvatarRole, "avatar"}};
}
