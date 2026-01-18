#include "delegators/messagedelegate.h"

#include <QVariant>

#include "models/messagemodel.h"
#include "utils.h"

namespace {

void fillCursorWithTokens(QTextCursor &cursor, DataManager &data_manager, const std::vector<MessageToken> &tokens,
                          int emojiSize) {  // todo: span??
  for (const auto &token : tokens) {
    if (token.type == MessageTokenType::Text) {
      cursor.insertText(token.value);
    } else if (token.type == MessageTokenType::Emoji) {
      DBC_REQUIRE(token.emoji_id.has_value());
      long long emojiId = token.emoji_id.value();
      auto img_info_opt = data_manager.getReactionInfo(emojiId);
      utils::ui::insert_emoji(cursor, img_info_opt, emojiSize);
    }
  }
}

}  // namespace

MessageDelegate::MessageDelegate(DataManager *data_manager, TokenManager *token_manager, QObject *parent)
    : QStyledItemDelegate(parent), data_manager_(data_manager), token_manager_(token_manager) {}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  if (!painter || !painter->isActive()) {
    qWarning() << "[Error] Painter error!";
    return;
  }

  auto message = [&]() -> Message {
    try {
      auto value_message = index.data(MessageModel::FullMessage);
      value_message.canConvert<Message>();
      return value_message.value<Message>();
    } catch (...) {
      throw std::runtime_error("Message in invalid in extractMessageData");
    }
  }();

  if (!message.isOfflineSaved()) {
    auto &boxes = hit_boxes_by_message_[message.id];
    boxes.clear();
  }

  painter->save();
  auto sender = extractSender(message.sender_id);
  drawAll(painter, option, message, sender);
  if (message.receiver_read_status == false) Q_EMIT unreadMessage(message);
  painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
  const QString text = index.data(MessageModel::TextRole).toString();
  constexpr int kTextFont = 12;
  const QFont font("Arial", kTextFont);
  const QFontMetrics fm(font);

  constexpr int kMinBubbleWidth = 80;
  constexpr int kAdditionalBubbleSpace = 10;
  const int maxBubbleWidth = option.rect.width() - kAdditionalBubbleSpace;

  const int bubble_width = qMax(kMinBubbleWidth, maxBubbleWidth);

  constexpr int kAvatarWidth = 30;
  constexpr int kPadding = 10;
  constexpr int kPaddingLeft = kAvatarWidth + kPadding;
  constexpr int kPaddingRight = kPadding;
  constexpr int kMinTextWidth = 20;
  int kAdditionalSpace = 80;

  auto message = [&]() -> Message {
    try {
      auto value_message = index.data(MessageModel::FullMessage);
      if (!value_message.canConvert<Message>()) {
        throw std::runtime_error("Cannot convert to Message");
      }
      return value_message.value<Message>();
    } catch (...) {
      throw std::runtime_error("Message in invalid in extractMessageData");
    }
  }();

  constexpr int replyHeight = 30;
  if (message.answer_on.has_value() && draw_answer_on) {
    kAdditionalSpace += replyHeight;
  }

  QFont textFont("Arial", 12);

  const int textWidth = qMax(bubble_width - kPaddingLeft - kPaddingRight, kMinTextWidth);
  const int height = calculateTextHeight(message.getFullText(), textWidth, textFont) + kAdditionalSpace;

  return {bubble_width, height};
}

int MessageDelegate::calculateTextHeight(const QString &text, int textWidth, const QFont &font) const {
  QTextDocument doc;
  doc.setDefaultFont(font);
  doc.setTextWidth(textWidth);
  doc.setPlainText(text);

  return qCeil(doc.size().height());
}

void MessageDelegate::drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option,
                                          bool is_mine) const {
  QColor bgColor = option.palette.color(QPalette::Base);
  QColor lighter = bgColor.lighter(110);
  painter->fillRect(rect, lighter);
}

void MessageDelegate::drawAvatar(QPainter *painter, const QRect &rect, const QPixmap &avatar, bool is_mine) const {
  constexpr int kAvatarSize = 30;

  auto avatar_rect = [&]() -> QRect {
    return is_mine ? QRect(rect.right() - 35, rect.top() + 5, kAvatarSize, kAvatarSize)
                   : QRect(rect.left() + 5, rect.top() + 5, kAvatarSize, kAvatarSize);
  };

  painter->drawPixmap(avatar_rect(),
                      avatar.scaled(kAvatarSize, kAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MessageDelegate::drawUsername(QPainter *painter, const QRect &rect, const QString &username, bool is_mine) const {
  painter->save();
  constexpr int kUsernameFont = 12;
  const QFont font("Arial", kUsernameFont, QFont::Bold);
  painter->setFont(font);

  const QColor nameColor = is_mine ? QColor("#0b8043") : QColor("#202124");
  painter->setPen(nameColor);
  constexpr int kSizeOffset = 55;
  constexpr int kTopOffset = 20;

  const int x = is_mine ? rect.right() - kSizeOffset - painter->fontMetrics().horizontalAdvance(username)
                        : rect.left() + kSizeOffset;

  painter->drawText(x, rect.top() + kTopOffset, username);
  painter->restore();
}

void MessageDelegate::drawText(QPainter *painter, const QRect &rect, const std::vector<MessageToken> &tokens,
                               bool is_mine) const {
  constexpr int kTextFont = 12;
  const int emojiSize = 14;

  painter->setFont(QFont("Arial", kTextFont));

  QTextDocument doc;
  QTextCursor cursor(&doc);
  doc.setTextWidth(rect.width() - 90);
  fillCursorWithTokens(cursor, *data_manager_, tokens, emojiSize);

  QTextBlockFormat blockFmt;
  blockFmt.setAlignment(is_mine ? Qt::AlignRight : Qt::AlignLeft);
  blockFmt.setLeftMargin(is_mine ? 55 : 20);
  blockFmt.setRightMargin(is_mine ? 20 : 55);
  cursor.select(QTextCursor::Document);
  cursor.setBlockFormat(blockFmt);

  painter->save();
  painter->translate(rect.left(), rect.top() + 40);
  doc.drawContents(painter, QRectF(0, 0, rect.width() - 90, rect.height() - 40));
  painter->restore();
}

// void MessageDelegate::drawTimestamp(QPainter *painter, const QRect &rect, const QDateTime &time, bool is_mine) const
// {
//   QString timestamp = time.toString("hh:mm dd.MM");
//   painter->setFont(QFont("Arial", 7));
//   QRect timeRect;

//   auto time_rect = [&]() {
//     return is_mine ? QRect(rect.left() + 10, rect.bottom() - 12, rect.width() - 15, 15)
//                    : QRect(rect.right() - 120, rect.bottom() - 20, 115, 15);
//   };

//   painter->drawText(time_rect(), is_mine ? Qt::AlignRight | Qt::AlignVCenter : Qt::AlignLeft | Qt::AlignVCenter,
//                     timestamp);
// }

void MessageDelegate::drawTimestamp(QPainter *painter, const QRect &rect, const QDateTime &time, bool is_mine) const {
  QString timestamp = time.toString("hh:mm dd.MM");
  painter->setFont(QFont("Arial", 7));

  QRect timeRect;
  constexpr int margin = 6;

  if (is_mine) {
    timeRect = QRect(rect.left() + margin, rect.bottom() - 12, rect.width() - 2 * margin, 12);
  } else {
    timeRect = QRect(rect.left() + margin, rect.bottom() - 12, rect.width() - 2 * margin, 12);
  }

  painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timestamp);
}

User MessageDelegate::extractSender(long long sender_id) const {
  if (auto sender = data_manager_->getUser(sender_id)) {
    sender->avatarPath = "/Users/roma/QtProjects/Chat/images/default_avatar.jpg";
    return *sender;
  }

  User default_user;
  default_user.name = "Unknown user";
  default_user.avatarPath = "/Users/roma/QtProjects/Chat/images/default_avatar.jpg";
  return default_user;
}

void MessageDelegate::drawAll(QPainter *painter, const QStyleOptionViewItem &option, const Message &msg,
                              const User &user) const {
  QRect fullRect = option.rect.normalized();

  constexpr int replyHeight = 30;

  QRect replyRect;
  QRect contentRect = fullRect;

  if (msg.answer_on.has_value() && draw_answer_on) {
    replyRect = QRect(fullRect.left(), fullRect.top(), fullRect.width(), replyHeight);

    contentRect =
        QRect(fullRect.left(), fullRect.top() + replyHeight, fullRect.width(), fullRect.height() - replyHeight);

    QColor color = option.palette.color(QPalette::Base);
    QColor darker = color.darker(110);
    drawAnswerOnStatus(painter, replyRect, darker, data_manager_->getMessageById(msg.answer_on.value()), msg.id);
  }

  bool is_mine = msg.isMine();

  drawBackgroundState(painter, contentRect, option, is_mine);
  drawAvatar(painter, contentRect, QPixmap(user.avatarPath), is_mine);
  drawUsername(painter, contentRect, user.name, is_mine);
  drawTimestamp(painter, contentRect, msg.timestamp, is_mine);

  drawText(painter, contentRect, msg.tokens, is_mine);
  drawStatus(painter, contentRect, msg.status_sended, msg.read_counter, is_mine);

  if (!msg.isOfflineSaved()) {
    DBC_REQUIRE(msg.isMine());
    drawReadCounter(painter, contentRect, msg.read_counter, is_mine);
    drawReactions(painter, contentRect, msg.reactions, msg.receiver_reaction, msg.id);
  }
}

void MessageDelegate::drawReactions(QPainter *painter, const QRect &rect,
                                    const std::unordered_map<long long, int> &reactions, std::optional<int> my_reaction,
                                    long long message_id) const {
  if (!draw_reactions) return;
  DBC_REQUIRE(message_id > 0);
  int offset = 30;
  for (const auto &[reaction_id, reaction_cnt] : reactions) {
    if (reaction_cnt <= 0) continue;
    auto reaction_info = data_manager_->getReactionInfo(reaction_id);
    if (!reaction_info.has_value()) {
      LOG_ERROR("No reaction_info for id {}", reaction_id);
      continue;
    }

    DBC_REQUIRE(reaction_info->checkInvariants());
    auto reaction_image_path = reaction_info->image;
    auto full_maked_icon =
        makeReactionIcon(QString::fromStdString(reaction_image_path), reaction_cnt, my_reaction, reaction_info->id);
    addInRect(painter, rect, full_maked_icon, reaction_info->id, message_id, offset);
  }
}

void MessageDelegate::drawStatus(QPainter *painter, const QRect &rect, bool is_sended, int read_cnt,
                                 bool is_mine) const {
  if (!is_mine) return;
  constexpr int status_size = 16;

  const QString status_symbol = !is_sended ? "!" : read_cnt > 1 ? ".." : ".";

  constexpr int kTopOffset = 30;
  constexpr int kLeftOffset = 4;

  QPoint position(rect.left() + kLeftOffset, rect.top() + kTopOffset);

  QFont font = painter->font();
  font.setPointSize(status_size);
  painter->setFont(font);

  painter->save();
  painter->setPen(QPen(Qt::gray));
  painter->drawText(position, status_symbol);
  painter->restore();
}

// void MessageDelegate::drawReadCounter(QPainter *painter, const QRect &rect, const int read_cnt, bool is_mine) const {
//   constexpr int kSize = 15;

//   constexpr int kTopOffset = 5;
//   constexpr int kLeftOffset = 10;

//   const int x = is_mine ? rect.left() + kLeftOffset / 2 : rect.right() - 4 * kLeftOffset;

//   const int y = is_mine ? rect.bottom() - kTopOffset * 2 : rect.top() + kTopOffset;

//   QRect circle_rect(x, y, kSize, kSize);
//   painter->save();

//   painter->setBrush(Qt::gray);
//   painter->setPen(Qt::NoPen);
//   painter->drawEllipse(circle_rect);

//   QFont font = painter->font();
//   font.setPixelSize(kSize - 4);
//   font.setBold(true);
//   painter->setFont(font);

//   painter->setPen(Qt::white);
//   painter->drawText(circle_rect, Qt::AlignCenter, QString::number(read_cnt));

//   painter->restore();
// }

void MessageDelegate::drawReadCounter(QPainter *painter, const QRect &rect, const int read_cnt, bool is_mine) const {
  constexpr int kSize = 15;
  constexpr int kTopOffset = 5;
  constexpr int kLeftOffset = 10;

  // Use rect.left/top/bottom relative to translated rectangle
  int x = is_mine ? rect.left() + kLeftOffset / 2 : rect.right() - kLeftOffset - kSize;
  int y = rect.bottom() - kTopOffset - kSize;  // always relative to bottom of rect

  QRect circle_rect(x, y, kSize, kSize);

  painter->save();
  painter->setBrush(Qt::gray);
  painter->setPen(Qt::NoPen);
  painter->drawEllipse(circle_rect);

  QFont font = painter->font();
  font.setPixelSize(kSize - 4);
  font.setBold(true);
  painter->setFont(font);

  painter->setPen(Qt::white);
  painter->drawText(circle_rect, Qt::AlignCenter, QString::number(read_cnt));

  painter->restore();
}

QPixmap MessageDelegate::makeReactionIcon(const QString &imagePath, int count, std::optional<int> my_reaction,
                                          int reaction_id) const {
  constexpr int iconSize = 20;
  constexpr int padding = 4;
  constexpr int badgeMinWidth = 14;
  constexpr int height = 24;

  QPixmap icon(imagePath);
  if (icon.isNull()) {
    icon = QPixmap(iconSize, iconSize);
    icon.fill(Qt::transparent);
  } else {
    icon = icon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }

  QFont font;
  font.setPixelSize(11);
  QFontMetrics fm(font);

  const QString text = QString::number(count);
  const int textWidth = std::max(badgeMinWidth, fm.horizontalAdvance(text));

  const int width = iconSize + padding + textWidth + padding;

  QPixmap result(width, height);
  result.fill(Qt::transparent);

  QPainter p(&result);
  p.setRenderHint(QPainter::Antialiasing);
  p.setFont(font);

  // highlight if my reaction
  if (my_reaction && *my_reaction == reaction_id) {
    p.setBrush(QColor(0, 120, 215, 60));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(result.rect(), 6, 6);
  }

  // draw icon
  p.drawPixmap(padding, (height - iconSize) / 2, icon);

  // draw count
  p.setPen(Qt::black);
  QRect textRect(iconSize + padding * 2, 0, textWidth, height);

  p.drawText(textRect, Qt::AlignCenter, text);

  return result;
}

void MessageDelegate::addInRect(QPainter *painter, const QRect &rect, const QPixmap &icon, int reaction_id,
                                long long message_id, int &reaction_x_offset) const {
  constexpr int spacing = 6;

  const int x = rect.left() + reaction_x_offset;
  const int y = rect.bottom() - icon.height() - 4;

  QRect hitRect(x, y, icon.width(), icon.height());

  painter->drawPixmap(hitRect.topLeft(), icon);

  DBC_REQUIRE(message_id > 0);
  DBC_REQUIRE(reaction_id > 0);
  hit_boxes_by_message_[message_id].push_back({hitRect, reaction_id});

  reaction_x_offset += icon.width() + spacing;
}

std::optional<int> MessageDelegate::reactionAt(long long message_id, const QPoint &pos) const {
  auto it = hit_boxes_by_message_.find(message_id);
  if (it == hit_boxes_by_message_.end()) return std::nullopt;

  for (const auto &box : it->second) {
    if (box.rect.contains(pos)) return box.reaction_id;
  }
  return std::nullopt;
}

void MessageDelegate::drawAnswerOnStatus(QPainter *painter, QRect &recte, const QColor &color,
                                         std::optional<Message> answer_on, long long message_id) const {
  if (!answer_on || !draw_answer_on) {
    DBC_UNREACHABLE();
    return;
  }

  Message &answer_on_message = answer_on.value();
  DBC_REQUIRE(!answer_on_message.isOfflineSaved());
  DBC_REQUIRE(answer_on_message.id != message_id);

  constexpr int replyHeight = 30;
  constexpr int padding = 6;
  constexpr int spacing = 2;
  constexpr int radius = 6;
  const int maxTextWidth = recte.width() - 90;

  recte.setHeight(recte.height() + replyHeight);

  QRect replyRect(recte.x(), recte.y(), recte.width(), replyHeight);
  QRect contentRect(recte.x(), recte.y() + replyHeight, recte.width(), recte.height() - replyHeight);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // Draw reply background
  painter->setBrush(color);
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(replyRect, radius, radius);

  // Prepare font
  QFont bodyFont = painter->font();
  bodyFont.setPointSize(bodyFont.pointSize() - 1);
  painter->setFont(bodyFont);

  // Draw reply text
  QFontMetrics bodyFm(bodyFont);
  QString body = answer_on_message.getFullText();
  body = bodyFm.elidedText(body, Qt::ElideRight, maxTextWidth);

  painter->setPen(QColor(60, 60, 60));
  painter->drawText(replyRect.adjusted(padding, padding + spacing, -padding, -padding), Qt::AlignLeft | Qt::AlignTop,
                    body);

  recte.translate(0, replyHeight);

  painter->restore();
}
