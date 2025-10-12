#include "messagedelegate.h"
#include "MessageModel/messagemodel.h"

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const{
    if (!painter || !painter->isActive()) {
        qWarning() << "[Error] Painter error!";
        return;
    }

    painter->save();
    auto msg = extractMessageData(index);
    drawAll(painter, option, msg);

    painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 60);
}

void MessageDelegate::drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option) const{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(rect, QColor("#d0e7ff"));
    else if (option.state & QStyle::State_MouseOver)
        painter->fillRect(rect, QColor("#f5f5f5"));
}

void MessageDelegate::drawAvatar(QPainter *painter, const QRect &rect, const QPixmap& avatar, const int senderId, const int receiverId) const{
    QRect avatarRect;
    avatarRect = QRect(rect.left() + 5, rect.top() + 5, 30, 30);
    painter->drawPixmap(avatarRect, avatar.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // if(senderId == receiverId) avatarRect = QRect(rect.right() - 35, rect.top() + 5, 30, 30);
}

void MessageDelegate::drawUsername(QPainter *painter, const QRect &rect, const QString &username) const{
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(rect.left() + 55, rect.top() + 20, username);
}

void MessageDelegate::drawText(QPainter *painter, const QRect &rect, const QString &text) const{
    painter->setFont(QFont("Arial", 9));
    painter->drawText(rect.left() + 55, rect.top() + 40, text);
}

void MessageDelegate::drawTimestamp(QPainter *painter, const QRect &rect, const QString &timestamp) const{
    painter->setFont(QFont("Arial", 8));
    QRect timeRect(rect.right() - 120, rect.bottom() - 20, 115, 15);
    painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timestamp);
}

MessageDrawData MessageDelegate::extractMessageData(const QModelIndex &index) const {
    MessageDrawData data;
    data.username = index.data(MessageModel::UsernameRole).toString();
    data.text = index.data(MessageModel::TextRole).toString();
    data.avatarPath = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
    data.timestamp = index.data(MessageModel::TimestampRole).toDateTime().toString("hh:mm dd.MM");
    data.senderId = index.data(MessageModel::SenderIdRole).toInt();
    data.receiverId = index.data(MessageModel::ReceiverIdTole).toInt();
    return data;
}

void MessageDelegate::drawAll(QPainter *painter, const QStyleOptionViewItem &option,
                              const MessageDrawData &msg) const {
    QRect rect = option.rect.normalized();
    drawBackgroundState(painter, rect, option);
    drawAvatar(painter, rect, QPixmap(msg.avatarPath), msg.senderId, msg.receiverId);
    drawUsername(painter, rect, msg.username);
    drawTimestamp(painter, rect, msg.timestamp);
    drawText(painter, rect, msg.text);
}
