#include "messagemodel.h"

//ChatModel::ChatModel() {}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();

    const auto &msg = m_messages[index.row()];

    switch (role) {
        case UsernameRole: return msgIdToUser.at(msg.id).name;
        case TextRole: return msg.text;
        case AvatarRole: return msgIdToUser.at(msg.id).avatarPath;
        case TimestampRole: return msg.timestamp;
        case SenderIdRole: return msg.senderId;
        case ReceiverIdTole: return msgIdToUser.at(msg.id).id;
        default: return QVariant();
    }
}
