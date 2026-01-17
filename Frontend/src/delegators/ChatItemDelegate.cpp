#include "delegators/chatitemdelegate.h"

#include "Utils.h"

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
  if(QVariant last_message_variant = index.data(ChatModel::LastMessageRole); last_message_variant.isValid()){
    data.last_message = last_message_variant.value<Message>();
  }

  data.avatar_path = index.data(ChatModel::AvatarRole).toString();
  data.unread = index.data(ChatModel::UnreadRole).toInt();
  return data;
}

void ChatItemDelegate::drawAll(QPainter *painter, const QStyleOptionViewItem &option, const ChatDrawData &chat) const {
  QRect rect = option.rect.normalized();

  drawBackgroundState(painter, rect, option);
  drawAvatar(painter, rect, QPixmap(chat.avatar_path));
  drawNameOfChat(painter, rect, chat.title);

  if(chat.last_message.has_value()) {
    const Message& last_message = chat.last_message.value();
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

void ChatItemDelegate::drawLastMessage(QPainter *painter, const QRect &rect, const std::optional<std::vector<MessageToken>>& tokens) const {
  QFont font(style_.last_message_font_style, style_.last_message_font_size);
  painter->setFont(font);

  if (!tokens.has_value() || tokens->empty()) {
    painter->drawText(rect.left() + 55, rect.top() + 40, "There is no text...");
    return;
  }

  QTextDocument doc;
  doc.setDefaultFont(QFont(style_.last_message_font_style, style_.last_message_font_size));
  int total_width = rect.width() - 90;
  doc.setTextWidth(total_width);
  QTextCursor cursor(&doc);

  QRectF drawRect(0, 0, rect.width() - 90, rect.height() - 40); // your fixed drawing area
  painter->save();
  painter->translate(rect.left(), rect.top() + 40);
  painter->setClipRect(drawRect);       // <-- clip everything outside
  doc.drawContents(painter, drawRect);  // only visible inside drawRect
  painter->restore();


  for (const auto& token : tokens.value()) {
      if (token.type == MessageTokenType::Text) {
          cursor.insertText(token.value);
      } else if (token.type == MessageTokenType::Emoji) {
        DBC_REQUIRE(token.emoji_id.has_value());
        long long emojiId = token.emoji_id.value();
        DBC_REQUIRE(emojiId > 0);
        auto img_info_opt = data_manager_->getReactionInfo(emojiId);
        utils::ui::insert_emoji(cursor, img_info_opt);
      }
  }

  QTextBlockFormat blockFmt;
  blockFmt.setAlignment(Qt::AlignLeft);
  blockFmt.setLeftMargin(55);
  blockFmt.setRightMargin(20);
  cursor.select(QTextCursor::Document);
  cursor.setBlockFormat(blockFmt);

  painter->save();
  painter->translate(rect.left(), rect.top() + 40);
  doc.drawContents(painter, QRectF(0, 0, rect.width() - 90, rect.height() - 40));
  painter->restore();

  // painter->setFont(QFont(style_.last_message_font_style, style_.last_message_font_size));
  // painter->drawText(rect.left() + 55, rect.top() + 40, text);
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

std::optional<std::vector<MessageToken>> ChatItemDelegate::getLastMessageTokens(const QString &msg) const {
  if (bool chat_empty = msg.isEmpty(); chat_empty == true) {
    return std::nullopt;
  }

  return utils::text::get_tokens_from_text(msg);
}

QSize ChatItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  return QSize(style_.chat_item_width, style_.chat_item_width);
}
