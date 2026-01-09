#ifndef DRAWDATA_H
#define DRAWDATA_H

#include <QDateTime>
#include <QRect>
#include <QString>

struct MessageDrawData {
  QString username;
  QString text;
  QString avatar_path;
  QString timestamp;
  long long sender_id;
  long long receiver_id;
  bool is_mine;
  bool is_sended;
  bool is_readed;
  int read_cnt;
};

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
