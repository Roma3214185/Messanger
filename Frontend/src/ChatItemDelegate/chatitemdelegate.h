#ifndef CHATITEMDELEGATE_H
#define CHATITEMDELEGATE_H

#include <QPainter>
#include <QStyledItemDelegate>

#include "ChatModel/chatmodel.h"
#include "headers/DrawData.h"

class ChatItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  using QStyledItemDelegate::QStyledItemDelegate;
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

 private:
  ChatDrawData extractChatData(const QModelIndex& index) const;
  void drawAll(QPainter* painter, const QStyleOptionViewItem& option,
               const ChatDrawData& chat) const;
  QString refactorLastMessage(const QString& msg) const;
  void drawBackgroundState(QPainter* painter, const QRect& rect,
                           const QStyleOptionViewItem& option) const;
  void drawAvatar(QPainter* painter, const QRect& rect,
                  const QPixmap& avatar) const;
  void drawNameOfChat(QPainter* painter, const QRect& rect,
                      const QString& title) const;
  void drawLastMessage(QPainter* painter, const QRect& rect,
                       const QString& text) const;
  void drawTimestamp(QPainter* painter, const QRect& rect,
                     const QDateTime& timestamp) const;
  void drawUnread(QPainter* painter, const QRect& rect, const int unread) const;
};

#endif  // CHATITEMDELEGATE_H
