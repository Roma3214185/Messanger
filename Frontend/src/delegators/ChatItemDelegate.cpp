#include "delegators/chatitemdelegate.h"

#include "utils.h"

ChatItemDelegate::ChatItemDelegate(DataManager *data_manager, QObject *parent, const ChatItemStyle &style)
    : data_manager_(data_manager), QStyledItemDelegate(parent), style_(style) {}

void ChatItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  painter->save();
  auto chat = extractChatData(index);
  drawAll(painter, option, chat);
  painter->restore();
}

void ChatItemDelegate::drawBackgroundState(QPainter *painter, const QRect & /*rect*/,
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
  if (QVariant last_message_variant = index.data(ChatModel::LastMessageRole); last_message_variant.isValid()) {
    data.last_message = last_message_variant.value<Message>();
  }

  data.avatar_path = index.data(ChatModel::AvatarRole).toString();
  data.unread = index.data(ChatModel::UnreadRole).toInt();
  return data;
}

void ChatItemDelegate::drawAll(QPainter *painter, const QStyleOptionViewItem &option, const ChatDrawData &chat) const {
  QRect rect = option.rect.normalized();
  rect.setHeight(65);

  drawBackgroundState(painter, rect, option);
  drawAvatar(painter, rect, QPixmap(chat.avatar_path));
  drawNameOfChat(painter, rect, chat.title);

  if (chat.last_message.has_value()) {
    const Message &last_message = chat.last_message.value();
    drawLastMessage(painter, rect, last_message.tokens);
    drawTimestamp(painter, rect, last_message.timestamp);
  } else {
    drawLastMessage(painter, rect, std::nullopt);
  }
  // drawUnread(painter, rect,  chat.unread);
}

void ChatItemDelegate::drawAvatar(QPainter *painter, const QRect &rect, const QPixmap &avatar) const {
  QRect avatar_rect(rect.left() + 5, rect.top() + 5, style_.avatar_size, style_.avatar_size);
  painter->drawPixmap(avatar_rect, avatar.scaled(style_.avatar_size, style_.avatar_size, Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
}

void ChatItemDelegate::drawNameOfChat(QPainter *painter, const QRect &rect, const QString &username) const {
  painter->setFont(QFont(style_.last_message_font_style, style_.last_message_font_size, QFont::Bold));
  painter->drawText(rect.left() + 55, rect.top() + 20, username);
}

void ChatItemDelegate::drawLastMessage(QPainter *painter, const QRect &rect,
                                       const std::optional<std::vector<MessageToken>> &tokens) const {
  QFont font(style_.last_message_font_style, style_.last_message_font_size);
  painter->setFont(font);

  if (!tokens || tokens->empty()) {
    painter->drawText(rect.left() + 55, rect.top() + 40, "There is no text...");
    return;
  }

  QTextDocument doc;
  doc.setDefaultFont(font);
  doc.setTextWidth(rect.width() - 90);

  QTextCursor cursor(&doc);

  for (const auto &token : *tokens) {
    if (token.type == MessageTokenType::Text) {
      cursor.insertText(token.value);
    } else if (token.type == MessageTokenType::Emoji) {
      DBC_REQUIRE(token.emoji_id.has_value());
      auto img_info_opt = data_manager_->getReactionInfo(*token.emoji_id);
      utils::ui::insert_emoji(cursor, img_info_opt);
    }
  }

  QTextBlockFormat blockFmt;
  blockFmt.setAlignment(Qt::AlignLeft);
  blockFmt.setLeftMargin(55);
  blockFmt.setRightMargin(20);

  cursor.select(QTextCursor::Document);
  cursor.setBlockFormat(blockFmt);

  QRectF drawRect(0, 0, rect.width() - 90, rect.height() - 40);

  painter->save();
  painter->translate(rect.left(), rect.top() + 40);
  // painter->setClipRect(drawRect);
  doc.drawContents(painter, drawRect);
  painter->restore();
}

void ChatItemDelegate::drawTimestamp(QPainter *painter, const QRect &rect, const QDateTime &timestamp) const {
  painter->setFont(QFont(style_.timestamp_font_style, style_.timestamp_font_size));
  QString time_str = timestamp.toString(style_.timestamp_format);
  painter->drawText(rect.right() - 50, rect.top() + 20, time_str);
}

void ChatItemDelegate::drawUnread(QPainter *painter, const QRect &rect, const int unread) const {
  if (unread <= 0) return;

  QRect circle_rect(rect.right() - 25, rect.center().y() - 10, style_.unread_label_size, style_.unread_label_size);
  painter->setBrush(Qt::red);
  painter->setPen(Qt::NoPen);
  painter->drawEllipse(circle_rect);
  painter->setPen(Qt::white);
  painter->setFont(QFont(style_.unread_font_style, style_.unread_label_font, QFont::Bold));
  painter->drawText(circle_rect, Qt::AlignCenter, QString::number(unread));
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  return QSize(style_.chat_item_width, style_.chat_item_width);
}
