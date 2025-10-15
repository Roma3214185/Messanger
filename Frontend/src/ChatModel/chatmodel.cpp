#include "chatmodel.h"

ChatModel::ChatModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int ChatModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return m_chats.size();
}

QVariant ChatModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid() || index.row() >= m_chats.size())
        return QVariant();

    const auto &chat = m_chats[index.row()];

    switch (role) {
        case ChatIdRole: return chat->chatId;
        case TitleRole: return chat->title;
        case LastMessageRole: return chat->lastMessage;
        case UnreadRole: return chat->unread;
        case LastMessageTimeRole: return chat->lastMessageTime;
        case AvatarRole: return chat->avatarPath;
        default: return QVariant();
    }
}

QHash<int, QByteArray> ChatModel::roleNames() const{
    return {
        {ChatIdRole, "chatId"},
        {TitleRole, "title"},
        {LastMessageRole, "lastMessage"},
        {UnreadRole, "unread"},
        {LastMessageTimeRole, "lastMessageTime"},
        {AvatarRole, "avatar"}
    };
}

void ChatModel::addChat(const ChatPtr& chat) {
    beginInsertRows(QModelIndex(), m_chats.size(), m_chats.size());
    m_chats.push_back(chat);
    qDebug() << "[INFO] addChat id = " << chat->chatId;
    endInsertRows();
}

void ChatModel::updateChat(const int chatId, const QString& lastMessage, const QDateTime& time /*, int unread = 0,*/) {
    for (int i = 0; i < m_chats.size(); i++) {
        if (m_chats[i]->chatId == chatId) {
            m_chats[i]->lastMessage = lastMessage;
            m_chats[i]->unread = 0; // unread++;
            m_chats[i]->lastMessageTime = time;
            QModelIndex idx = index(i);
            Q_EMIT dataChanged(idx, idx);
            qDebug() << "[INFO] updatedChat id = " << chatId;
            return;
        }
    }
}

void ChatModel::addChatInFront(const ChatPtr& chat){
    beginInsertRows(QModelIndex(), m_chats.size(), m_chats.size());
    m_chats.push_front(chat);
    qDebug() << "[INFO] Add chat infront id = " << chat->chatId;
    endInsertRows();
}

void ChatModel::realocateChatInFront(const int chatId){
    auto index = findIndexByChatId(chatId);
    if (!index) return;

    bool chatIsFirstInList = *index == 0;
    if(chatIsFirstInList) return;

    beginMoveRows(QModelIndex(), *index, *index, QModelIndex(), 0);
    auto chat = m_chats.takeAt(*index);
    m_chats.prepend(chat);
    qDebug() << "[INFO] Realocate chat infront id = " << chat->chatId;
    endMoveRows();
}

OptionalChatIndex ChatModel::findIndexByChatId(const int chatId) const{
    for (size_t i = 0; i < m_chats.size(); ++i) {
        if (m_chats[i]->chatId == chatId) {
            return i;
        }
    }

    return std::nullopt;
}

void ChatModel::clear(){
    if (m_chats.isEmpty()) return;

    beginRemoveRows(QModelIndex(), 0, m_chats.size() - 1);
    m_chats.clear();
    endRemoveRows();
}
