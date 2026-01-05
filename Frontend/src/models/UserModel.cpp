#include "models/UserModel.h"

#include "Debug_profiling.h"

UserModel::UserModel(QObject* parent) : QAbstractListModel(parent) {}

int UserModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return users_.size();
}

void UserModel::addUser(const User& user) {
  if (user.id <= 0) throw std::runtime_error("Invalid user id");

  for (auto existing_user : users_) {
    if (existing_user.id == user.id) {
      LOG_ERROR("User with id {} already exist", user.id);
      return;
    }
  }

  beginInsertRows(QModelIndex(), users_.size(), users_.size());
  LOG_INFO(
      "User model add user ({}) with email: {}", user.name.toStdString(), user.email.toStdString());
  users_.push_back(user);
  endInsertRows();
}

void UserModel::clear() {
  beginResetModel();
  users_.clear();
  endResetModel();
}

QVariant UserModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= users_.size() || index.row() < 0) return QVariant();

  const User& user = users_[index.row()];
  switch (role) {
    case UserModel::Roles::UserIdRole:
      return user.id;
    case UserModel::Roles::NameRole:
      return user.name;
    case UserModel::Roles::TagRole:
      return user.tag;
    case UserModel::Roles::EmailRole:
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
          {UserModel::Roles::EmailRole, "email"},
          {UserModel::Roles::AvatarRole, "avatar"}};
}
