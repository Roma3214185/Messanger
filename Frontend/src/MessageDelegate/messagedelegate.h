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

    void drawAll(QPainter *painter, const QStyleOptionViewItem &option, const MessageDrawData &msg, bool isMine) const;
    MessageDrawData extractMessageData(const QModelIndex &index) const;
    void drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option, bool isMine) const;
    void drawAvatar(QPainter *painter, const QRect &rect, const QPixmap& avatar, bool isMine) const;
    void drawUsername(QPainter *painter, const QRect &rect, const QString &username, bool isMine) const;
    void drawText(QPainter *painter, const QRect &rect, const QString &text, bool isMine) const;
    void drawTimestamp(QPainter *painter, const QRect &rect, const QString &timestamp, bool isMine) const;
};


#endif // CHATDELEGATE_H
