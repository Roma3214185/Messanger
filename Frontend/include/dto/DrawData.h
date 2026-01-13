#ifndef DRAWDATA_H
#define DRAWDATA_H

#include <QDateTime>
#include <QRect>
#include <QString>

struct UserDrawData {
  QString name;
  QPixmap avatar;
  QString tag;
};

struct ChatDrawData {
  QString title;
  QString last_message;
  QString avatar_path;
  QDateTime time;
  int unread;
};

#endif  // DRAWDATA_H
