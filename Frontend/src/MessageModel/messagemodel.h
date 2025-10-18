#pragma once
#include <QAbstractItemModel>
#include <QDateTime>
#include <QPixmap>
#include "UserModel/UserModel.h"
#include "Model/model.h"
#include "headers/Message.h"

using ListOfMessages = QList<Message>;
using MessageId = int;
using UsersByMessageId = std::unordered_map<MessageId, User>;

class MessageModel : public QAbstractListModel
{
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

    MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addMessage(Message msg, const User& user);
    void addMessageInBack(Message msg, const User& user);
    void clear();
    std::optional<Message> getLastMessage();
    std::optional<Message> getFirstMessage();
    static void setCurrentUserId(int id);
    void resetCurrentUseId();


private:

    ListOfMessages messages_;
    UsersByMessageId usersByMessageId;
    static std::optional<int> currentUserId;
};

