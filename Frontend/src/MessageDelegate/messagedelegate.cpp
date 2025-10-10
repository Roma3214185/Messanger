#include "messagedelegate.h"
#include "MessageModel/messagemodel.h"

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const{
            if (!painter)
                return;

            if (!painter->isActive()) {
                qWarning() << "[Error] Painter is not active!";
                return;
            }

            painter->save();

            QRect rect = option.rect.normalized();

            QString username = index.data(MessageModel::UsernameRole).toString();
            QString text = index.data(MessageModel::TextRole).toString();
            QString avatarPath = "/Users/roma/QtProjects/Chat/default_avatar.jpeg"; //fix
            QString timestamp = index.data(MessageModel::TimestampRole).toDateTime().toString("hh:mm dd.MM");

            int senderId = index.data(MessageModel::SenderIdRole).toInt();
            int receiverId = index.data(MessageModel::ReceiverIdTole).toInt();

            QPixmap avatar(avatarPath);
            if (avatar.isNull())
                avatar = QPixmap(40, 40); // set default image

            if (option.state & QStyle::State_Selected)
                painter->fillRect(rect, QColor("#d0e7ff"));
            else if (option.state & QStyle::State_MouseOver)
                painter->fillRect(rect, QColor("#f5f5f5"));

            QRect avatarRect;
            // if (senderId != receiverId)
            //     avatarRect = QRect(rect.left() + 5, rect.top() + 5, 30, 30);
            // else
            //     avatarRect = QRect(rect.right() - 35, rect.top() + 5, 30, 30);

            avatarRect = QRect(rect.left() + 5, rect.top() + 5, 30, 30);
            painter->drawPixmap(avatarRect, avatar.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            painter->setFont(QFont("Arial", 10, QFont::Bold));
            painter->drawText(rect.left() + 55, rect.top() + 20, username);

            painter->setFont(QFont("Arial", 9));
            painter->drawText(rect.left() + 55, rect.top() + 40, text);


            //right and down timestamp
            painter->setFont(QFont("Arial", 8));
            QRect timeRect(rect.right() - 120, rect.bottom() - 20, 115, 15);
            painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timestamp);

            painter->restore();
    }

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 60); // висота одного елемента
}
