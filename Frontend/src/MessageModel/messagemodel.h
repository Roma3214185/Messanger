#pragma once

#include <QAbstractItemModel>
#include <QDateTime>
#include <QPixmap>
#include "UserModel/UserModel.h"
#include "Model/model.h"

struct Message {
    int id;
    int senderId;
    int chatId;
    QString text;
    QDateTime timestamp = QDateTime::currentDateTime();
};


class MessageModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        UsernameRole = Qt::UserRole + 1,
        TextRole,
        AvatarRole,
        TimestampRole,
        ReceiverIdTole,
        SenderIdRole
    };

    MessageModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_messages.size();
    }

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override {
        return {
            {UsernameRole, "username"},
            {TextRole, "text"},
            {AvatarRole, "avatar"},
            {TimestampRole, "timestamp"},
            {ReceiverIdTole, "receiver_id"},
            {SenderIdRole, "sender_id"}
        };
    }

    void addMessage(const Message &msg, const User& user) {
        qDebug() << "[INFO] addMessage" << msg.text << " and id = " << msg.id;
        auto it = msgIdToUser.find(msg.id);

        if(it != msgIdToUser.end()) {    // undelivered messages will not copy when i load all messages (fix updating unread); load ONLY readen by u (?)
            qDebug() << "MSG EXIST";
            return;
        }

        beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
        m_messages.push_back(msg); //track receiver_id and check if sender_id == receiver_id???
        msgIdToUser[msg.id] = user;
        endInsertRows();
    }

    void clear(){
        if (m_messages.isEmpty())
            return;

        beginRemoveRows(QModelIndex(), 0, m_messages.size() - 1);
        m_messages.clear();
        msgIdToUser.clear();
        endRemoveRows();
    }

private:
    QVector<Message> m_messages;
    std::unordered_map<int, User> msgIdToUser;
};

