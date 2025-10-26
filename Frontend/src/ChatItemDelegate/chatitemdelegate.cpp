#include "chatitemdelegate.h"

void ChatItemDelegate::paint(
    QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    painter->save();
    auto chat = extractChatData(index);
    drawAll(painter, option, chat);
    painter->restore();
}

void ChatItemDelegate::drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option) const {
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
    data.last_message = index.data(ChatModel::LastMessageRole).toString();
    data.avatar_path = index.data(ChatModel::AvatarRole).toString();
    data.time = index.data(ChatModel::LastMessageTimeRole).toDateTime();
    data.unread = index.data(ChatModel::UnreadRole).toInt();
    return data;
}

void ChatItemDelegate::drawAll(
    QPainter *painter,
    const QStyleOptionViewItem &option,
    const ChatDrawData &chat) const
{
    QRect rect = option.rect.normalized();
    QString refactored_last_message = refactorLastMessage(chat.last_message);

    drawBackgroundState(painter, rect, option);
    drawAvatar(painter, rect, QPixmap(chat.avatar_path));
    drawNameOfChat(painter, rect, chat.title);
    drawLastMessage(painter,rect, refactored_last_message);
    drawTimestamp(painter, rect, chat.time);
    //drawUnread(painter, rect,  chat.unread);
}

void ChatItemDelegate::drawAvatar(QPainter *painter, const QRect &rect, const QPixmap& avatar) const {
    constexpr int k_avatar_size = 40;
    QRect avatar_rect(rect.left() + 5, rect.top() + 5, k_avatar_size, k_avatar_size);
    painter->drawPixmap(avatar_rect, avatar.scaled(k_avatar_size, k_avatar_size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ChatItemDelegate::drawNameOfChat(QPainter *painter, const QRect &rect, const QString &username) const {
    painter->setFont(QFont("Arial", 12, QFont::Bold));
    painter->drawText(rect.left() + 55, rect.top() + 20, username);
}

void ChatItemDelegate::drawLastMessage(QPainter *painter, const QRect &rect, const QString &text) const {
    painter->setFont(QFont("Arial", 9));
    painter->drawText(rect.left() + 55, rect.top() + 40, text);
}

void ChatItemDelegate::drawTimestamp(QPainter *painter, const QRect &rect, const QDateTime &timestamp) const {
    constexpr int k_timestamp_font = 8;
    painter->setFont(QFont("Arial", k_timestamp_font));
    QString time_str = timestamp.toString("hh:mm");
    painter->drawText(rect.right() - 50, rect.top() + 20, time_str);
}

void ChatItemDelegate::drawUnread(QPainter *painter, const QRect &rect, const int unread) const {
    if(unread <= 0) return;

    constexpr int k_unread_label_size = 20;
    constexpr int k_unread_label_font = 8;

    QRect circle_rect(rect.right() - 25, rect.center().y() - 10, k_unread_label_size, k_unread_label_size);
    painter->setBrush(Qt::red);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle_rect);
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", k_unread_label_font, QFont::Bold));
    painter->drawText(circle_rect, Qt::AlignCenter, QString::number(unread));
}

QString ChatItemDelegate::refactorLastMessage(const QString& msg) const {
    bool chat_empty = msg.isEmpty();
    if(chat_empty) {
        return "There is no messages";
    }

    constexpr int max_len = 25;
    constexpr int num_dots = 3;

    if(msg.length() >= max_len) {
        return msg.left(max_len - num_dots) + QString(num_dots, QChar('.'));
    }

    return msg;
}

QSize ChatItemDelegate::sizeHint(
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    constexpr int width = 250;
    constexpr int height = 60;
    return QSize(width, height);
}
