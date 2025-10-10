#ifndef USERDELEGATE_H
#define USERDELEGATE_H

#include <QStyledItemDelegate>
#include <QPixmap>
#include <QPainter>
#include <UserModel/UserModel.h>

class UserDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        painter->save();
        QRect rect = option.rect;

        // Дані з моделі
        QString name = index.data(UserModel::NameRole).toString();
        QString avatarPath = index.data(UserModel::AvatarRole).toString();
        qDebug() << avatarPath;
        QString tag = index.data(UserModel::TagRole).toString();

        QPixmap avatar(avatarPath);


        // Фон
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(rect, QColor("#d0e7ff"));
        } else if (option.state & QStyle::State_MouseOver) {
            painter->fillRect(rect, QColor("#f5f5f5"));
        }

        // Аватарка
        QRect avatarRect(rect.left() + 5, rect.top() + 5, 40, 40);
        painter->drawPixmap(avatarRect, avatar.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // Назва чату
        painter->setFont(QFont("Arial", 10, QFont::Bold));
        painter->drawText(rect.left() + 55, rect.top() + 20, name);

        //tag
        painter->setFont(QFont("Arial", 9));
        painter->drawText(rect.left() + 55, rect.top() + 40, tag);

        // // Час
        // painter->setFont(QFont("Arial", 8));
        // QString timeStr = time.toString("hh:mm");
        // painter->drawText(rect.right() - 50, rect.top() + 20, timeStr);

        // // Останнє повідомлення
        // painter->setFont(QFont("Arial", 9));
        // painter->drawText(rect.left() + 55, rect.top() + 40, lastMessage);

        // // Непрочитані
        // if (unread > 0) {
        //     QRect circleRect(rect.right() - 25, rect.center().y() - 10, 20, 20);
        //     painter->setBrush(Qt::red);
        //     painter->setPen(Qt::NoPen);
        //     painter->drawEllipse(circleRect);

        //     painter->setPen(Qt::white);
        //     painter->setFont(QFont("Arial", 8, QFont::Bold));
        //     painter->drawText(circleRect, Qt::AlignCenter, QString::number(unread));
        // }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(250, 60);
    }
};

#endif // USERDELEGATE_H
