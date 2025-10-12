#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include "MessageModel/messagemodel.h"
#include <QFile>
#include <QDateTime>
#include <QVariant>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include "headers/DrawData.h"

class MessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:

    void drawAll(QPainter *painter, const QStyleOptionViewItem &option, const MessageDrawData &msg) const;
    MessageDrawData extractMessageData(const QModelIndex &index) const;
    void drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option) const;
    void drawAvatar(QPainter *painter, const QRect &rect, const QPixmap& avatar, const int senderId, const int receiverId) const;
    void drawUsername(QPainter *painter, const QRect &rect, const QString &username) const;
    void drawText(QPainter *painter, const QRect &rect, const QString &text) const;
    void drawTimestamp(QPainter *painter, const QRect &rect, const QString &timestamp) const;
};


#endif // CHATDELEGATE_H
