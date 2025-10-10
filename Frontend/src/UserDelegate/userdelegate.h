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

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void drawAvatar(QPainter *painter, const QRect &rect, const QPixmap &avatar) const;
    void drawName(QPainter *painter, const QRect &rect, const QString &name) const;
    void drawTag(QPainter *painter, const QRect &rect, const QString &tag) const;
    void drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option) const;
};

#endif // USERDELEGATE_H
