#include "UserModel.h"

UserModel::UserModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int UserModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return m_users.size();
}

void UserModel::addUser(const User &user) {
    beginInsertRows(QModelIndex(), m_users.size(), m_users.size());
    m_users.push_back(user);
    endInsertRows();
}

void UserModel::clear() {
    beginResetModel();
    m_users.clear();
    endResetModel();
}

QVariant UserModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_users.size())
        return QVariant();

    const auto &user = m_users[index.row()];
    switch (role) {
        case UserIdRole: return user.id;
        case NameRole: return user.name;
        case TagRole: return user.tag;
        case EmailTimeRole: return user.email;
        case AvatarRole: return user.avatarPath;
        default: return QVariant();
    }
}

QHash<int, QByteArray> UserModel::roleNames() const {
    return {
        {UserIdRole, "chat_id"},
        {NameRole, "name"},
        {TagRole, "tag"},
        {EmailTimeRole, "email"},
        {AvatarRole, "avatar"}
    };
}

