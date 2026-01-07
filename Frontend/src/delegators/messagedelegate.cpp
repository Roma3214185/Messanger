#include "delegators/messagedelegate.h"

#include <QVariant>

#include "models/messagemodel.h"

void MessageDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
  if (!painter || !painter->isActive()) {
    qWarning() << "[Error] Painter error!";
    return;
  }

  auto message = [&]() -> Message {
    auto value_message = index.data(MessageModel::FullMessage);
    if (value_message.canConvert<Message>()) {
      return value_message.value<Message>();
    }

    throw std::runtime_error("Message in invalid in extractMessageData");
  }();

  painter->save();
  auto draw_message_data = extractMessageData(message);
  const bool is_mine =
      draw_message_data.sender_id == draw_message_data.receiver_id;
  drawAll(painter, option, draw_message_data, is_mine);
  if (draw_message_data.is_readed == false)
    Q_EMIT unreadMessage(message);
  painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
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
  constexpr int kAdditionalSpace = 50;

  const int textWidth =
      qMax(bubble_width - kPaddingLeft - kPaddingRight, kMinTextWidth);

  const QRect text_rect =
      fm.boundingRect(0, 0, textWidth, 0, Qt::TextWordWrap, text);
  const int height = text_rect.height() + kAdditionalSpace;
  return {bubble_width, height};
}

void MessageDelegate::drawBackgroundState(QPainter *painter, const QRect &rect,
                                          const QStyleOptionViewItem &option,
                                          bool is_mine) const {
  // const QColor kLightBlue = QColor("#d0e7ff");
  // if (option.state & QStyle::State_Selected)
  //   painter->fillRect(rect, kLightBlue);
}

void MessageDelegate::drawAvatar(QPainter *painter, const QRect &rect,
                                 const QPixmap &avatar, bool is_mine) const {
  constexpr int kAvatarSize = 30;

  auto avatar_rect = [&]() -> QRect {
    return is_mine ? QRect(rect.right() - 35, rect.top() + 5, kAvatarSize,
                           kAvatarSize)
                   : QRect(rect.left() + 5, rect.top() + 5, kAvatarSize,
                           kAvatarSize);
  };

  painter->drawPixmap(avatar_rect(), avatar.scaled(kAvatarSize, kAvatarSize,
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));
}

void MessageDelegate::drawUsername(QPainter *painter, const QRect &rect,
                                   const QString &username,
                                   bool is_mine) const {
  painter->save();
  constexpr int kUsernameFont = 12;
  const QFont font("Arial", kUsernameFont, QFont::Bold);
  painter->setFont(font);

  const QColor nameColor = is_mine ? QColor("#0b8043") : QColor("#202124");
  painter->setPen(nameColor);
  constexpr int kSizeOffset = 55;
  constexpr int kTopOffset = 20;

  const int x = is_mine ? rect.right() - kSizeOffset -
                              painter->fontMetrics().horizontalAdvance(username)
                        : rect.left() + kSizeOffset;

  painter->drawText(x, rect.top() + kTopOffset, username);
  painter->restore();
}

void MessageDelegate::drawText(QPainter *painter, const QRect &rect,
                               const QString &text, bool is_mine) const {
  constexpr int kTextFont = 12;
  painter->setFont(QFont("Arial", kTextFont));

  auto text_rect = [&]() {
    return is_mine ? QRect(rect.left() + 20, rect.top() + 40, rect.width() - 90,
                           rect.height() - 40)
                   : QRect(rect.left() + 55, rect.top() + 40, rect.width() - 90,
                           rect.height() - 40);
  };

  painter->drawText(
      text_rect(),
      (is_mine ? Qt::AlignRight : Qt::AlignLeft) | Qt::TextWordWrap, text);
}

void MessageDelegate::drawTimestamp(QPainter *painter, const QRect &rect,
                                    const QString &timestamp,
                                    bool is_mine) const {
  painter->setFont(QFont("Arial", 7));
  QRect timeRect;

  auto time_rect = [&]() {
    return is_mine ? QRect(rect.left() + 10, rect.bottom() - 12,
                           rect.width() - 15, 15)
                   : QRect(rect.right() - 120, rect.bottom() - 20, 115, 15);
  };

  painter->drawText(time_rect(),
                    is_mine ? Qt::AlignRight | Qt::AlignVCenter
                            : Qt::AlignLeft | Qt::AlignVCenter,
                    timestamp);
}

MessageDrawData
MessageDelegate::extractMessageData(const Message &message) const {
  auto user = [&]() -> User {
    std::optional<User> getted_user = data_manager_->getUser(message.sender_id);
    if (getted_user) {
      return *getted_user;
    }

    User default_user;
    default_user.name = "Unknown user";
    return default_user;
  }();

  MessageDrawData data;
  data.username = user.name;
  data.text = message.text;
  data.avatar_path = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  data.timestamp = message.timestamp.toString("hh:mm dd.MM");
  data.sender_id = message.sender_id;
  data.receiver_id = token_manager_->getCurrentUserId();
  data.is_sended = message.status_sended;
  data.is_readed = message.readed_by_me;
  data.read_cnt = message.read_counter;
  return data;
}

void MessageDelegate::drawAll(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const MessageDrawData &msg, bool is_mine) const {
  QRect rect = option.rect.normalized();
  drawBackgroundState(painter, rect, option, is_mine);
  drawAvatar(painter, rect, QPixmap(msg.avatar_path), is_mine);
  drawUsername(painter, rect, msg.username, is_mine);
  drawTimestamp(painter, rect, msg.timestamp, is_mine);
  drawText(painter, rect, msg.text, is_mine);
  drawStatus(painter, rect, msg, is_mine);
  drawReadCounter(painter, rect, msg.read_cnt, is_mine);
}

void MessageDelegate::drawStatus(QPainter *painter, const QRect &rect,
                                 const MessageDrawData &message_data,
                                 bool is_mine) const {
  if (!is_mine)
    return;
  constexpr int status_size = 16;

  const auto status_symbol = [message_data]() -> QString {
    return !message_data.is_sended ? "!" : message_data.is_readed ? ".." : ".";
  };

  constexpr int kTopOffset = 30;
  constexpr int kLeftOffset = 4;

  QPoint position(rect.left() + kLeftOffset, rect.top() + kTopOffset);

  QFont font = painter->font();
  font.setPointSize(status_size);
  painter->setFont(font);

  painter->save();
  painter->setPen(QPen(Qt::gray));
  painter->drawText(position, status_symbol());
  painter->restore();
}

void MessageDelegate::drawReadCounter(QPainter *painter, const QRect &rect,
                                      const int read_cnt, bool is_mine) const {
  constexpr int kSize = 15;

  constexpr int kTopOffset = 5;
  constexpr int kLeftOffset = 10;

  const int x =
      is_mine ? rect.left() + kLeftOffset / 2 : rect.right() - 4 * kLeftOffset;

  const int y =
      is_mine ? rect.bottom() - kTopOffset * 2 : rect.top() + kTopOffset;

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
