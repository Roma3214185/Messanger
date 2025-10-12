#include "messagemodel.h"

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= messages_.size())
        return QVariant();

    const auto &msg = messages_[index.row()];

    switch (role) {
        case UsernameRole: return usersByMessageId.at(msg.id).name;
        case TextRole: return msg.text;
        case AvatarRole: return usersByMessageId.at(msg.id).avatarPath;
        case TimestampRole: return msg.timestamp;
        case SenderIdRole: return msg.senderId;
        case ReceiverIdTole: return usersByMessageId.at(msg.id).id;
        default: return QVariant();
    }
}

void MessageModel::addMessage(Message msg, const User& user) {
    auto it = usersByMessageId.find(msg.id);

    if(it != usersByMessageId.end()) {
        // undelivered messages will not copy when i load all messages;
        qDebug() << "[WARN] Message with id " << msg.text << "(id = " << msg.id << ") already exist";
        return;
    }

    beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
    messages_.push_back(msg); //track receiver_id and check if sender_id == receiver_id???
    usersByMessageId[msg.id] = user;
    endInsertRows();
}

void MessageModel::clear(){
    if (messages_.isEmpty()){
        return;
    }

    beginRemoveRows(QModelIndex(), 0, messages_.size() - 1);
    messages_.clear();
    usersByMessageId.clear();
    endRemoveRows();
}

QHash<int, QByteArray> MessageModel::roleNames() const{
    return {
        {UsernameRole, "username"},
        {TextRole, "text"},
        {AvatarRole, "avatar"},
        {TimestampRole, "timestamp"},
        {ReceiverIdTole, "receiver_id"},
        {SenderIdRole, "sender_id"}
    };
}

int MessageModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return messages_.size();
}
