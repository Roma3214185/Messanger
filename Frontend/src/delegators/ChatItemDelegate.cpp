#include "delegators/chatitemdelegate.h"

void ChatItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  painter->save();
  auto chat = extractChatData(index);
  drawAll(painter, option, chat);
  painter->restore();
}

void ChatItemDelegate::drawBackgroundState(
    QPainter *painter, const QRect & /*rect*/,
    const QStyleOptionViewItem &option) const {
  if (option.state != 0u && QStyle::State_Selected) {
    const QColor bg = option.palette.color(QPalette::Base);
    int r = qMin(bg.red() + 38, 255);
    int g = qMin(bg.green() + 38, 255);
    int b = qMin(bg.blue() + 38, 255);
    painter->fillRect(option.rect, QColor(r, g, b));
  }
}

ChatDrawData ChatItemDelegate::extractChatData(const QModelIndex &index) const {
  ChatDrawData data;
  data.title = index.data(ChatModel::TitleRole).toString();
  data.last_message = index.data(ChatModel::LastMessageRole).toString();
  data.avatar_path = index.data(ChatModel::AvatarRole).toString();
  data.time = index.data(ChatModel::LastMessageTimeRole).toDateTime();
  data.unread = index.data(ChatModel::UnreadRole).toInt();
  return data;
}

void ChatItemDelegate::drawAll(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const ChatDrawData &chat) const {
  QRect rect = option.rect.normalized();
  QString refactored_last_message = refactorLastMessage(chat.last_message);

  drawBackgroundState(painter, rect, option);
  drawAvatar(painter, rect, QPixmap(chat.avatar_path));
  drawNameOfChat(painter, rect, chat.title);
  drawLastMessage(painter, rect, refactored_last_message);
  drawTimestamp(painter, rect, chat.time);
  // drawUnread(painter, rect,  chat.unread);
}

void ChatItemDelegate::drawAvatar(QPainter *painter, const QRect &rect,
                                  const QPixmap &avatar) const {
  QRect avatar_rect(rect.left() + 5, rect.top() + 5, style_.avatar_size,
                    style_.avatar_size);
  painter->drawPixmap(avatar_rect,
                      avatar.scaled(style_.avatar_size, style_.avatar_size,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
}

void ChatItemDelegate::drawNameOfChat(QPainter *painter, const QRect &rect,
                                      const QString &username) const {
  painter->setFont(QFont(style_.last_message_font_style,
                         style_.last_message_font_size, QFont::Bold));
  painter->drawText(rect.left() + 55, rect.top() + 20, username);
}

void ChatItemDelegate::drawLastMessage(QPainter *painter, const QRect &rect,
                                       const QString &text) const {
  painter->setFont(
      QFont(style_.last_message_font_style, style_.last_message_font_size));
  painter->drawText(rect.left() + 55, rect.top() + 40, text);
}

void ChatItemDelegate::drawTimestamp(QPainter *painter, const QRect &rect,
                                     const QDateTime &timestamp) const {
  painter->setFont(
      QFont(style_.timestamp_font_style, style_.timestamp_font_size));
  QString time_str = timestamp.toString(style_.timestamp_format);
  painter->drawText(rect.right() - 50, rect.top() + 20, time_str);
}

void ChatItemDelegate::drawUnread(QPainter *painter, const QRect &rect,
                                  const int unread) const {
  if (unread <= 0)
    return;

  QRect circle_rect(rect.right() - 25, rect.center().y() - 10,
                    style_.unread_label_size, style_.unread_label_size);
  painter->setBrush(Qt::red);
  painter->setPen(Qt::NoPen);
  painter->drawEllipse(circle_rect);
  painter->setPen(Qt::white);
  painter->setFont(
      QFont(style_.unread_font_style, style_.unread_label_font, QFont::Bold));
  painter->drawText(circle_rect, Qt::AlignCenter, QString::number(unread));
}

QString ChatItemDelegate::refactorLastMessage(const QString &msg) const {
  bool chat_empty = msg.isEmpty();
  if (chat_empty) {
    return style_.no_message_status;
  }

  if (msg.length() >= style_.max_message_len) {
    return msg.left(style_.max_message_len - style_.num_dots) +
           QString(style_.num_dots, QChar('.'));
  }

  return msg;
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  return QSize(style_.chat_item_width, style_.chat_item_width);
}
