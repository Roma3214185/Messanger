#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H

#include <QDateTime>
#include <QFile>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

#include "dto/DrawData.h"
#include "managers/TokenManager.h"
#include "models/messagemodel.h"

class MessageDelegate : public QStyledItemDelegate {
  Q_OBJECT
  DataManager *data_manager_;
  TokenManager *token_manager_;

 public:
  MessageDelegate(DataManager *data_manager, TokenManager *token_manager)
      : data_manager_(data_manager), token_manager_(token_manager) {}

  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

 private:
  void drawAll(QPainter *painter, const QStyleOptionViewItem &option, const MessageDrawData &msg, bool is_mine) const;
  [[nodiscard]] MessageDrawData extractMessageData(const Message &index) const;
  void drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option,
                           bool is_mine) const;
  void drawAvatar(QPainter *painter, const QRect &rect, const QPixmap &avatar, bool is_mine) const;
  void drawUsername(QPainter *painter, const QRect &rect, const QString &username, bool is_mine) const;
  void drawText(QPainter *painter, const QRect &rect, const QString &text, bool is_mine) const;
  void drawTimestamp(QPainter *painter, const QRect &rect, const QString &timestamp, bool is_mine) const;
  void drawStatus(QPainter *painter, const QRect &rect, const MessageDrawData &message_data, bool is_mine) const;

  void drawReadCounter(QPainter *painter, const QRect &rect, const int read_cnt, bool is_mine) const;

 Q_SIGNALS:
  void unreadMessage(Message &message) const;
  // todo: this signal have to me in QlistView
};

#endif  // CHATDELEGATE_H
