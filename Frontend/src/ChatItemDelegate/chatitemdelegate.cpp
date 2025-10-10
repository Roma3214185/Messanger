#include "chatitemdelegate.h"

void ChatItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    painter->save();
    QRect rect = option.rect;
    const QWidget *widget = option.widget;

    if (widget) {
        int listWidth = widget->width();
        rect.setWidth(listWidth);
    }

    auto title = index.data(ChatModel::TitleRole).toString();
    auto lastMessage = index.data(ChatModel::LastMessageRole).toString();
    auto avatarPath = index.data(ChatModel::AvatarRole).toString();
    auto time = index.data(ChatModel::LastMessageTimeRole).toDateTime();
    auto unread = index.data(ChatModel::UnreadRole).toInt();
    QPixmap avatar(avatarPath);

    auto refactoredLastMessage = refactorLastMessge(lastMessage);

    drawBackgroundState(painter, rect, option);
    drawAvatar(painter, rect, avatar);
    drawNameOfChat(painter, rect, title);
    drawLastMessage(painter,rect, refactoredLastMessage);
    drawTimestamp(painter, rect, time);
    drawUnread(painter, rect,  unread);

    painter->restore();
}

void ChatItemDelegate::drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option) const{
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, QColor("#d0e7ff"));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, QColor("#f5f5f5"));
    }
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
    if(msg == "") return "There is no messages";

    const int mxLenOfMessage = 25;
    const int numsOfDots = 3;

    if(msg.length() >= mxLenOfMessage) {
        std::string temp = msg.toStdString();
        temp = temp.substr(0, mxLenOfMessage - numsOfDots);
        temp += std::string(numsOfDots, '.');
        return QString::fromStdString(temp);
    }

    return msg;
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(250, 60);
}
