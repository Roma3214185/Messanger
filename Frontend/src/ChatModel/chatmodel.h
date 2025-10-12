#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QPixmap>

#include "headers/ChatBase.h"

using ChatPtr = std::shared_ptr<ChatBase>;
using ListOfChats = QList<ChatPtr>;
using std::optional;
using ChatIndex = size_t;
using OptionalChatIndex = std::optional<ChatIndex>;

class ChatModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum Roles {
        ChatIdRole = Qt::UserRole + 1,
        TitleRole,
        LastMessageRole,
        UnreadRole,
        LastMessageTimeRole,
        AvatarRole
    };

    ChatModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addChat(const ChatPtr& chat);
    void updateChat(const int chatId, const QString& lastMessage, const QDateTime& time /*, int unread = 0,*/);
    void addChatInFront(const ChatPtr &chat);
    void realocateChatInFront(const int chatId);
    void clear();

private:

    OptionalChatIndex findIndexByChatId(const int chatId);

    ListOfChats m_chats;
};


#endif // CHATMODEL_H
