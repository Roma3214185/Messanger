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

class MessageDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

};


#endif // CHATDELEGATE_H
