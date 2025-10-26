#include "UserModel.h"

UserModel::UserModel(QObject* parent) : QAbstractListModel(parent) {}
UserModel::UserModel() : UserModel(nullptr) {}

int UserModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return m_users.size();
}

int UserModel::rowCount() const { return rowCount(QModelIndex()); }

void UserModel::addUser(const User& user) {
  beginInsertRows(QModelIndex(), m_users.size(), m_users.size());
  qDebug() << "User model add user " << user.name << " email: " << user.email;
  m_users.push_back(user);
  endInsertRows();
}

void UserModel::clear() {
  beginResetModel();
  m_users.clear();
  endResetModel();
}

QVariant UserModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_users.size()) return QVariant();

  const User& user = m_users[index.row()];
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
