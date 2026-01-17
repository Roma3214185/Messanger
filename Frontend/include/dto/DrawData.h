#ifndef DRAWDATA_H
#define DRAWDATA_H

#include <QDateTime>
#include <QRect>
#include <QString>

#include <dto/Message.h>

struct UserDrawData {
  QString name;
  QPixmap avatar;
  QString tag;
};

struct ChatDrawData {
  QString title;
  std::optional<Message> last_message;
  QString avatar_path;
  int unread;
};

#endif  // DRAWDATA_H
