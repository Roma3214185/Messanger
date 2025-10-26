#ifndef USERDELEGATE_H
#define USERDELEGATE_H

#include <QPainter>
#include <QPixmap>
#include <QStyledItemDelegate>

#include "UserModel/UserModel.h"
#include "headers/DrawData.h"

class UserDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

 private:
  void drawAll(QPainter* painter, const QStyleOptionViewItem& option,
               const UserDrawData& user) const;
  UserDrawData extractMessageData(const QModelIndex& index) const;
  void drawAvatar(QPainter* painter, const QRect& rect,
                  const QPixmap& avatar) const;
  void drawName(QPainter* painter, const QRect& rect,
                const QString& name) const;
  void drawTag(QPainter* painter, const QRect& rect, const QString& tag) const;
  void drawBackgroundState(QPainter* painter, const QRect& rect,
                           const QStyleOptionViewItem& option) const;
};

#endif  // USERDELEGATE_H
