#include "chatitemdelegate.h"

void ChatItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    painter->save();
    auto chat = extractChatData(index);
    drawAll(painter, option, chat);

    painter->restore();
}

void ChatItemDelegate::drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option) const{
    static const QColor BLUE(208, 231, 255);
    static const QColor GRAY(245, 245, 245);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, BLUE);
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, GRAY);
    }
}

ChatDrawData ChatItemDelegate::extractChatData(const QModelIndex &index) const {
    ChatDrawData data;
    data.title = index.data(ChatModel::TitleRole).toString();
    data.lastMessage = index.data(ChatModel::LastMessageRole).toString();
    data.avatarPath = index.data(ChatModel::AvatarRole).toString();
    data.time = index.data(ChatModel::LastMessageTimeRole).toDateTime();
    int unread = index.data(ChatModel::UnreadRole).toInt();
    return data;
}

void ChatItemDelegate::drawAll(QPainter *painter, const QStyleOptionViewItem &option,
                           const ChatDrawData &chat) const {
    QRect rect = option.rect.normalized();
    QString refactoredLastMessage = refactorLastMessge(chat.lastMessage);

    drawBackgroundState(painter, rect, option);
    drawAvatar(painter, rect, QPixmap(chat.avatarPath));
    drawNameOfChat(painter, rect, chat.title);
    drawLastMessage(painter,rect, refactoredLastMessage);
    drawTimestamp(painter, rect, chat.time);
    //drawUnread(painter, rect,  chat.unread);
}

void ChatItemDelegate::drawAvatar(QPainter *painter, const QRect &rect, const QPixmap& avatar) const{
    QRect avatarRect(rect.left() + 5, rect.top() + 5, 40, 40);
    painter->drawPixmap(avatarRect, avatar.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ChatItemDelegate::drawNameOfChat(QPainter *painter, const QRect &rect, const QString &username) const{
    painter->setFont(QFont("Arial", 12, QFont::Bold));
    painter->drawText(rect.left() + 55, rect.top() + 20, username);
}

void ChatItemDelegate::drawLastMessage(QPainter *painter, const QRect &rect, const QString &text) const{
    painter->setFont(QFont("Arial", 9));
    painter->drawText(rect.left() + 55, rect.top() + 40, text);
}

void ChatItemDelegate::drawTimestamp(QPainter *painter, const QRect &rect, const QDateTime &timestamp) const{
    painter->setFont(QFont("Arial", 8));
    QString timeStr = timestamp.toString("hh:mm");
    painter->drawText(rect.right() - 50, rect.top() + 20, timeStr);
}

void ChatItemDelegate::drawUnread(QPainter *painter, const QRect &rect, const int unread) const{
    if (unread > 0) {
        QRect circleRect(rect.right() - 25, rect.center().y() - 10, 20, 20);
        painter->setBrush(Qt::red);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(circleRect);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(circleRect, Qt::AlignCenter, QString::number(unread));
    }
}

QString ChatItemDelegate::refactorLastMessge(const QString& msg) const{
    if(msg.isEmpty()) return "There is no messages";

    constexpr int maxLen = 25;
    constexpr int numDots = 3;

    if(msg.length() >= maxLen) {
        return msg.left(maxLen - numDots) + QString(numDots, QChar('.'));
    }

    return msg;
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(250, 60);
}
