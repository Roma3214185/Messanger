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

    UserModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addUser(const User &user);
    void clear();

private:
    ListOfUsers m_users;
};

#endif // USERMODEL_H
