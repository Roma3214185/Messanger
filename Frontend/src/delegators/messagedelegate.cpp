#include "delegators/messagedelegate.h"

#include <QVariant>

#include "models/messagemodel.h"

void MessageDelegate::paint(QPainter*                   painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex&          index) const {
  if (!painter || !painter->isActive()) {
    qWarning() << "[Error] Painter error!";
    return;
  }

  painter->save();
  auto msg     = extractMessageData(index);
  bool is_mine = msg.sender_id == msg.receiver_id;
  drawAll(painter, option, msg, is_mine);
  painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex&          index) const {
  QString       text      = index.data(MessageModel::TextRole).toString();
  constexpr int kTextFont = 12;
  QFont         font("Arial", kTextFont);
  QFontMetrics  fm(font);

  constexpr int kMinBubbleWidth        = 80;
  constexpr int kAdditionalBubbleSpace = 10;
  int           maxBubbleWidth         = option.rect.width() - kAdditionalBubbleSpace;

  int bubbleWidth = qMax(kMinBubbleWidth, maxBubbleWidth);

  constexpr int kAvatarWidth     = 30;
  constexpr int kPadding         = 10;
  constexpr int kPaddingLeft     = kAvatarWidth + kPadding;
  constexpr int kPaddingRight    = kPadding;
  constexpr int kMinTextWidth    = 20;
  constexpr int kAdditionalSpace = 50;

  int textWidth = bubbleWidth - kPaddingLeft - kPaddingRight;
  if (textWidth < kMinTextWidth) textWidth = kMinTextWidth;

  QRect textRect = fm.boundingRect(0, 0, textWidth, 0, Qt::TextWordWrap, text);
  int   height   = textRect.height() + kAdditionalSpace;
  return QSize(bubbleWidth, height);
}

void MessageDelegate::drawBackgroundState(QPainter*                   painter,
                                          const QRect&                rect,
                                          const QStyleOptionViewItem& option,
                                          bool                        isMine) const {
  // const QColor kLightBlue = QColor("#d0e7ff");
  // if (option.state & QStyle::State_Selected)
  //   painter->fillRect(rect, kLightBlue);
}

void MessageDelegate::drawAvatar(QPainter*      painter,
                                 const QRect&   rect,
                                 const QPixmap& avatar,
                                 bool           is_mine) const {
  QRect         avatarRect;
  constexpr int kAvatarSize = 30;

  if (is_mine) {
    avatarRect = QRect(rect.right() - 35, rect.top() + 5, kAvatarSize, kAvatarSize);
  } else {
    avatarRect = QRect(rect.left() + 5, rect.top() + 5, kAvatarSize, kAvatarSize);
  }

  painter->drawPixmap(avatarRect,
                      avatar.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MessageDelegate::drawUsername(QPainter*      painter,
                                   const QRect&   rect,
                                   const QString& username,
                                   bool           is_mine) const {
  painter->save();
  constexpr int kUsernameFont = 12;
  QFont         font("Arial", kUsernameFont, QFont::Bold);
  painter->setFont(font);

  QColor nameColor = is_mine ? QColor("#0b8043") : QColor("#202124");
  painter->setPen(nameColor);
  constexpr int kSizeOffset = 55;
  constexpr int kTopOffset  = 20;

  int x = is_mine ? rect.right() - kSizeOffset - painter->fontMetrics().horizontalAdvance(username)
                  : rect.left() + kSizeOffset;

  painter->drawText(x, rect.top() + kTopOffset, username);
  painter->restore();
}

void MessageDelegate::drawText(QPainter*      painter,
                               const QRect&   rect,
                               const QString& text,
                               bool           is_mine) const {
  constexpr int kTextFont = 12;
  painter->setFont(QFont("Arial", kTextFont));

  QRect textRect;
  if (is_mine)
    textRect = QRect(rect.left() + 20, rect.top() + 40, rect.width() - 90, rect.height() - 40);
  else
    textRect = QRect(rect.left() + 55, rect.top() + 40, rect.width() - 90, rect.height() - 40);

  painter->drawText(textRect, (is_mine ? Qt::AlignRight : Qt::AlignLeft) | Qt::TextWordWrap, text);
}

void MessageDelegate::drawTimestamp(QPainter*      painter,
                                    const QRect&   rect,
                                    const QString& timestamp,
                                    bool           is_mine) const {
  painter->setFont(QFont("Arial", 7));
  QRect timeRect;

  if (is_mine)
    timeRect = QRect(rect.left() + 10, rect.bottom() - 12, rect.width() - 15, 15);
  else
    timeRect = QRect(rect.right() - 120, rect.bottom() - 20, 115, 15);

  painter->drawText(timeRect,
                    is_mine ? Qt::AlignRight | Qt::AlignVCenter : Qt::AlignLeft | Qt::AlignVCenter,
                    timestamp);
}

MessageDrawData MessageDelegate::extractMessageData(const QModelIndex& index) const {
  MessageDrawData data;
  data.username    = index.data(MessageModel::UsernameRole).toString();
  data.text        = index.data(MessageModel::TextRole).toString();
  data.avatar_path = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  data.timestamp   = index.data(MessageModel::TimestampRole).toDateTime().toString("hh:mm dd.MM");
  data.sender_id   = index.data(MessageModel::SenderIdRole).toInt();
  data.receiver_id = index.data(MessageModel::ReceiverIdTole).toInt();
  data.is_sended   = index.data(MessageModel::SendedStatusRole).toInt();
  data.is_readed   = index.data(MessageModel::ReadedStatusRole).toInt();
  return data;
}

void MessageDelegate::drawAll(QPainter*                   painter,
                              const QStyleOptionViewItem& option,
                              const MessageDrawData&      msg,
                              bool                        is_mine) const {
  QRect rect = option.rect.normalized();
  drawBackgroundState(painter, rect, option, is_mine);
  drawAvatar(painter, rect, QPixmap(msg.avatar_path), is_mine);
  drawUsername(painter, rect, msg.username, is_mine);
  drawTimestamp(painter, rect, msg.timestamp, is_mine);
  drawText(painter, rect, msg.text, is_mine);
  drawStatus(painter, rect, msg, is_mine);
}

void MessageDelegate::drawStatus(QPainter*              painter,
                                 const QRect&           rect,
                                 const MessageDrawData& message_data,
                                 bool                   is_mine) const {
  if (!is_mine) return;
  QString       status_symbol;
  constexpr int status_size = 16;

  if (!message_data.is_sended) {
    status_symbol = "!";
  } else if (message_data.is_readed) {
    status_symbol = "..";
  } else {
    status_symbol = ".";
  }

  constexpr int kTopOffset  = 30;
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
