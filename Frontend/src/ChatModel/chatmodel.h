#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QPixmap>

struct ChatBase {
    int chatId;
    QString title;
    QString lastMessage;
    int unread = 0;
    QDateTime lastMessageTime;
    QString avatarPath;   // або QByteArray avatarData
    virtual ~ChatBase() = default;
    virtual bool isPrivate() const = 0;
};

struct PrivateChat : public ChatBase {
    QString userTag;
    int userId;
    QString status; // наприклад: "online", "last seen recently"
    bool isPrivate() const override { return true; }
};

struct GroupChat : public ChatBase {
    int memberCount = 0;
    QVector<QString> memberTags;
    QVector<int> membersId;
    QVector<QString> avatarPaths; // кілька аватарок або колаж
    bool isPrivate() const override { return false; }
};


class ChatModel : public QAbstractListModel {
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

    ChatModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_chats.size();
    }

    QVariant data(const QModelIndex &index, int role) const override {
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

    QHash<int, QByteArray> roleNames() const override {
        return {
            {ChatIdRole, "chatId"},
            {TitleRole, "title"},
            {LastMessageRole, "lastMessage"},
            {UnreadRole, "unread"},
            {LastMessageTimeRole, "lastMessageTime"},
            {AvatarRole, "avatar"}
        };
    }

    // Додавання чату
    void addChat(const std::shared_ptr<ChatBase> &chat) {
        beginInsertRows(QModelIndex(), m_chats.size(), m_chats.size());
        m_chats.push_back(chat);
        endInsertRows();
    }

    // Оновлення чату (наприклад, нове повідомлення)
    void updateChat(int chatId, const QString &lastMessage, const QDateTime &time /*, int unread = 0,*/) {
        qDebug() << "[INFO] updateChat" << lastMessage;

        for (int i = 0; i < m_chats.size(); i++) {
            if (m_chats[i]->chatId == chatId) {
                m_chats[i]->lastMessage = lastMessage;
                m_chats[i]->unread = 0; // unread++;
                m_chats[i]->lastMessageTime = time;
                QModelIndex idx = index(i);
                Q_EMIT dataChanged(idx, idx); // повідомляємо View оновити елемент
                break;
            }
        }
    }

    void addChatInFront(const std::shared_ptr<ChatBase> &chat){
        beginInsertRows(QModelIndex(), m_chats.size(), m_chats.size());
        m_chats.push_front(chat);
        endInsertRows();
    }

    void realocateChatInFront(int chatId){
        int index = -1;
        for (int i = 0; i < m_chats.size(); ++i) {
            if (m_chats[i]->chatId == chatId) {
                index = i;
                break;
            }
        }
        if (index <= 0) return; // вже на початку або не знайдено

        beginMoveRows(QModelIndex(), index, index, QModelIndex(), 0);
        auto chat = m_chats.takeAt(index);
        m_chats.prepend(chat);
        endMoveRows();
    }

    void clear(){
        if (m_chats.isEmpty())
            return;

        beginRemoveRows(QModelIndex(), 0, m_chats.size() - 1);
        m_chats.clear();
        endRemoveRows();
    }


private:
    QVector<std::shared_ptr<ChatBase>> m_chats;
};


#endif // CHATMODEL_H
