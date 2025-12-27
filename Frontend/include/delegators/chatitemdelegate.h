#ifndef CHATITEMDELEGATE_H
#define CHATITEMDELEGATE_H

#include <QPainter>
#include <QStyledItemDelegate>

#include "dto/DrawData.h"
#include "models/chatmodel.h"

struct ChatItemStyle {
    int avatar_size          = 40;
    int name_font_size        = 12;
    int last_message_font_size = 9;
    int timestamp_font_size   = 8;
    int unread_label_size     = 20;
    int unread_label_font     = 8;
    int padding_left         = 5;
    int padding_top          = 5;
    int chat_item_width       = 250;
    int chat_item_height      = 60;
    int max_message_len       = 25;
    int num_dots             = 3;
    QString timestamp_format = "hh:mm";
    QString timestamp_font_style = "Arial";
    QString last_message_font_style = "Arial";
    QString name_font_style = "Arial";
    QString unread_font_style = "Arial";
    QString no_message_status = "There is no messages";
};

class ChatItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit ChatItemDelegate(QObject* parent = nullptr, ChatItemStyle style = ChatItemStyle())
      : QStyledItemDelegate(parent), style_(style) {}

  void  paint(QPainter*                   painter,
              const QStyleOptionViewItem& option,
              const QModelIndex&          index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

 protected:
  [[nodiscard]] ChatDrawData extractChatData(const QModelIndex& index) const;
  void         drawAll(QPainter*                   painter,
                       const QStyleOptionViewItem& option,
                       const ChatDrawData&         chat) const;
  [[nodiscard]] QString      refactorLastMessage(const QString& msg) const;
  void         drawBackgroundState(QPainter*                   painter,
                                   const QRect&                rect,
                                   const QStyleOptionViewItem& option) const;
  void         drawAvatar(QPainter* painter, const QRect& rect, const QPixmap& avatar) const;
  void         drawNameOfChat(QPainter* painter, const QRect& rect, const QString& title) const;
  void         drawLastMessage(QPainter* painter, const QRect& rect, const QString& text) const;
  void drawTimestamp(QPainter* painter, const QRect& rect, const QDateTime& timestamp) const;
  void drawUnread(QPainter* painter, const QRect& rect, const int unread) const;

 private:
  ChatItemStyle style_;
};

#endif  // CHATITEMDELEGATE_H
