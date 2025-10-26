#include "userdelegate.h"

void UserDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
  painter->save();
  auto user = extractMessageData(index);
  drawAll(painter, option, user);
  painter->restore();
}

QSize UserDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);
  return QSize(250, 60);
}

void UserDelegate::drawAvatar(QPainter* painter, const QRect& rect,
                              const QPixmap& avatar) const {
  QRect avatarRect(rect.left() + 5, rect.top() + 5, 40, 40);
  painter->drawPixmap(avatarRect, avatar.scaled(40, 40, Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}

void UserDelegate::drawName(QPainter* painter, const QRect& rect,
                            const QString& name) const {
  painter->setFont(QFont("Arial", 10, QFont::Bold));
  painter->drawText(rect.left() + 55, rect.top() + 20, name);
}

void UserDelegate::drawTag(QPainter* painter, const QRect& rect,
                           const QString& tag) const {
  painter->setFont(QFont("Arial", 9));
  painter->drawText(rect.left() + 55, rect.top() + 40, tag);
}

void UserDelegate::drawBackgroundState(
    QPainter* painter, const QRect& rect,
    const QStyleOptionViewItem& option) const {
  if (option.state & QStyle::State_Selected) {
    painter->fillRect(rect, QColor("#d0e7ff"));
  } else if (option.state & QStyle::State_MouseOver) {
    painter->fillRect(rect, QColor("#f5f5f5"));
  }
}

UserDrawData UserDelegate::extractMessageData(const QModelIndex& index) const {
  UserDrawData data;
  data.name = index.data(UserModel::NameRole).toString();
  data.avatar = QPixmap(index.data(UserModel::AvatarRole).toString());
  data.tag = index.data(UserModel::TagRole).toString();
  return data;
}

void UserDelegate::drawAll(QPainter* painter,
                           const QStyleOptionViewItem& option,
                           const UserDrawData& user) const {
  QRect rect = option.rect.normalized();
  drawBackgroundState(painter, rect, option);
  drawAvatar(painter, rect, user.avatar);
  drawName(painter, rect, user.name);
  drawTag(painter, rect, user.tag);
}
