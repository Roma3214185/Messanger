#include "messagedelegate.h"
#include "MessageModel/messagemodel.h"

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const
{
    if (!painter || !painter->isActive()) {
        qWarning() << "[Error] Painter error!";
        return;
    }

    painter->save();

    QRect rect = option.rect.normalized();

    auto username = index.data(MessageModel::UsernameRole).toString();
    auto text = index.data(MessageModel::TextRole).toString();
    auto avatarPath = "/Users/roma/QtProjects/Chat/default_avatar.jpeg"; //fix
    auto timestamp = index.data(MessageModel::TimestampRole).toDateTime().toString("hh:mm dd.MM");
    auto senderId = index.data(MessageModel::SenderIdRole).toInt();
    auto receiverId = index.data(MessageModel::ReceiverIdTole).toInt();
    QPixmap avatar(avatarPath);

    drawBackgroundState(painter, rect, option);
    drawAvatar(painter, rect, avatar, senderId, receiverId);
    drawUsername(painter, rect, username);
    drawTimestamp(painter, rect, timestamp);
    drawText(painter, rect, text);

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
