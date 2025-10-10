#ifndef USERMODEL_H
#define USERMODEL_H

#include <QAbstractListModel>
#include "headers/User.h"


class UserModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        UserIdRole = Qt::UserRole + 1,
        NameRole,
        TagRole,
        EmailTimeRole,
        AvatarRole
    };

    UserModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_users.size();
    }

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addUser(const User &user);
    void clear();

private:
    QVector<User> m_users;
};

#endif // USERMODEL_H
