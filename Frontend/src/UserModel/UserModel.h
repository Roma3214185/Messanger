#ifndef USERMODEL_H
#define USERMODEL_H

#include <QAbstractListModel>

#include "headers/User.h"

class UserModel : public QAbstractListModel {
  Q_OBJECT
 public:
  using ListOfUsers = QVector<User>;

  enum Roles {
    UserIdRole = Qt::UserRole + 1,
    NameRole,
    TagRole,
    EmailTimeRole,
    AvatarRole
  };

  explicit UserModel(QObject* parent);
  UserModel();

  [[nondiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
  [[nondiscard]] auto rowCount() const -> int;
  [[nondiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
  [[nondiscard]]auto roleNames() const ->  QHash<int, QByteArray> override;
  void addUser(const User& user);
  void clear();

 private:
  ListOfUsers m_users;
};

#endif  // USERMODEL_H
