#include "delegators/userdelegate.h"

void UserDelegate::paint(QPainter*                   painter,
                         const QStyleOptionViewItem& option,
                         const QModelIndex&          index) const {
  painter->save();
  auto user = extractMessageData(index);
  drawAll(painter, option, user);
  painter->restore();
}

QSize UserDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  constexpr int kWidth  = 250;
  constexpr int kHeight = 250;
  return QSize(kWidth, kHeight);
}

void UserDelegate::drawAvatar(QPainter* painter, const QRect& rect, const QPixmap& avatar) const {
  constexpr int kAvatarSize = 40;
  QRect         avatarRect(rect.left() + 5, rect.top() + 5, kAvatarSize, kAvatarSize);
  painter->drawPixmap(
      avatarRect,
      avatar.scaled(kAvatarSize, kAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void UserDelegate::drawName(QPainter* painter, const QRect& rect, const QString& name) const {
  constexpr int kNameFont = 10;
  painter->setFont(QFont("Arial", kNameFont, QFont::Bold));
  painter->drawText(rect.left() + 55, rect.top() + 20, name);
}

void UserDelegate::drawTag(QPainter* painter, const QRect& rect, const QString& tag) const {
  constexpr int kTagFont = 9;
  painter->setFont(QFont("Arial", kTagFont));
  painter->drawText(rect.left() + 55, rect.top() + 40, tag);
}

void UserDelegate::drawBackgroundState(QPainter*                   painter,
                                       const QRect&                rect,
                                       const QStyleOptionViewItem& option) const {
  if (option.state & QStyle::State_Selected) {
    QColor bg = option.palette.color(QPalette::Base);
    int    r  = qMin(bg.red() + 38, 255);
    int    g  = qMin(bg.green() + 38, 255);
    int    b  = qMin(bg.blue() + 38, 255);
    painter->fillRect(option.rect, QColor(r, g, b));
  }
}

UserDrawData UserDelegate::extractMessageData(const QModelIndex& index) const {
  UserDrawData data;
  data.name   = index.data(UserModel::NameRole).toString();
  data.avatar = QPixmap(index.data(UserModel::AvatarRole).toString());
  data.tag    = index.data(UserModel::TagRole).toString();
  return data;
}

void UserDelegate::drawAll(QPainter*                   painter,
                           const QStyleOptionViewItem& option,
                           const UserDrawData&         user) const {
  QRect rect = option.rect.normalized();
  drawBackgroundState(painter, rect, option);
  drawAvatar(painter, rect, user.avatar);
  drawName(painter, rect, user.name);
  drawTag(painter, rect, user.tag);
}
