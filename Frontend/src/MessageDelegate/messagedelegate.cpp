#include "messagedelegate.h"

#include <QVariant>

#include "MessageModel/messagemodel.h"

void MessageDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const {
  if (!painter || !painter->isActive()) {
    qWarning() << "[Error] Painter error!";
    return;
  }

  painter->save();
  auto msg = extractMessageData(index);
  bool isMine = msg.sender_id == msg.receiver_id;
  drawAll(painter, option, msg, isMine);
  painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
  QString text = index.data(MessageModel::TextRole).toString();
  QFont font("Arial", 12);
  QFontMetrics fm(font);

  int minBubbleWidth = 80;
  int maxBubbleWidth = option.rect.width() - 10;  // залежить від вікна

  int bubbleWidth = qMax(minBubbleWidth, maxBubbleWidth);

  int avatarWidth = 30;
  int padding = 10;
  int paddingLeft = avatarWidth + padding;
  int paddingRight = padding;

  // Текст повинен вміщатися всередині бульбашки
  int textWidth = bubbleWidth - paddingLeft - paddingRight;
  if (textWidth < 20) textWidth = 20;  // мінімальна ширина тексту

  QRect textRect = fm.boundingRect(0, 0, textWidth, 0, Qt::TextWordWrap, text);

  int height = textRect.height() + 50;  // враховує username, padding, аватар

  return QSize(bubbleWidth, height);
}

void MessageDelegate::drawBackgroundState(QPainter* painter, const QRect& rect,
                                          const QStyleOptionViewItem& option,
                                          bool isMine) const {
  if (option.state & QStyle::State_Selected)
    painter->fillRect(rect, QColor("#d0e7ff"));
  else if (option.state & QStyle::State_MouseOver)
    painter->fillRect(rect, QColor("#f5f5f5"));
  else {
    // QColor bgColor = isMine ? QColor("#dcf8c6") : QColor("#ffffff");
    // QRect bubbleRect = rect.adjusted(isMine ? 80 : 10, 5, isMine ? -10 : -80,
    // -5); painter->setBrush(bgColor); painter->setPen(Qt::NoPen);
    // painter->drawRoundedRect(bubbleRect, 10, 10);
  }
}

void MessageDelegate::drawAvatar(QPainter* painter, const QRect& rect,
                                 const QPixmap& avatar, bool isMine) const {
  QRect avatarRect;

  if (isMine)
    avatarRect = QRect(rect.right() - 35, rect.top() + 5, 30, 30);
  else
    avatarRect = QRect(rect.left() + 5, rect.top() + 5, 30, 30);

  painter->drawPixmap(avatarRect, avatar.scaled(30, 30, Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}

void MessageDelegate::drawUsername(QPainter* painter, const QRect& rect,
                                   const QString& username, bool isMine) const {
  painter->save();

  QFont font("Arial", 12, QFont::Bold);
  painter->setFont(font);

  QColor nameColor = isMine ? QColor("#0b8043") : QColor("#202124");
  painter->setPen(nameColor);

  int x = isMine ? rect.right() - 55 -
                       painter->fontMetrics().horizontalAdvance(username)
                 : rect.left() + 55;

  painter->drawText(x, rect.top() + 20, username);
  painter->restore();
}

void MessageDelegate::drawText(QPainter* painter, const QRect& rect,
                               const QString& text, bool isMine) const {
  painter->setFont(QFont("Arial", 12));

  QRect textRect;
  if (isMine)
    textRect = QRect(rect.left() + 20, rect.top() + 40, rect.width() - 90,
                     rect.height() - 40);
  else
    textRect = QRect(rect.left() + 55, rect.top() + 40, rect.width() - 90,
                     rect.height() - 40);

  painter->drawText(
      textRect, (isMine ? Qt::AlignRight : Qt::AlignLeft) | Qt::TextWordWrap,
      text);
}

void MessageDelegate::drawTimestamp(QPainter* painter, const QRect& rect,
                                    const QString& timestamp,
                                    bool isMine) const {
  painter->setFont(QFont("Arial", 7));
  QRect timeRect;

  if (isMine)
    timeRect =
        QRect(rect.left() + 10, rect.bottom() - 12, rect.width() - 15, 15);
  else
    timeRect = QRect(rect.right() - 120, rect.bottom() - 20, 115, 15);

  painter->drawText(timeRect,
                    isMine ? Qt::AlignRight | Qt::AlignVCenter
                           : Qt::AlignLeft | Qt::AlignVCenter,
                    timestamp);
}

MessageDrawData MessageDelegate::extractMessageData(
    const QModelIndex& index) const {
  MessageDrawData data;
  data.username = index.data(MessageModel::UsernameRole).toString();
  data.text = index.data(MessageModel::TextRole).toString();
  data.avatar_path = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  data.timestamp = index.data(MessageModel::TimestampRole)
                       .toDateTime()
                       .toString("hh:mm dd.MM");
  data.sender_id = index.data(MessageModel::SenderIdRole).toInt();
  data.receiver_id = index.data(MessageModel::ReceiverIdTole).toInt();
  return data;
}

void MessageDelegate::drawAll(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const MessageDrawData& msg, bool isMine) const {
  QRect rect = option.rect.normalized();
  drawBackgroundState(painter, rect, option, isMine);
  drawAvatar(painter, rect, QPixmap(msg.avatar_path), isMine);
  drawUsername(painter, rect, msg.username, isMine);
  drawTimestamp(painter, rect, msg.timestamp, isMine);
  drawText(painter, rect, msg.text, isMine);
}
