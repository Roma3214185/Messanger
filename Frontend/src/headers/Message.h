#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>

struct Message {
  int id;
  int senderId;
  int chatId;
  QString text;
  QDateTime timestamp;
  bool readed_by_me;
  bool liked_by_me;
  int read_counter = 0;
  int liked_counter =
      0;  // if less than 3 -> get this id's and get these avatars
};

#endif  // MESSAGE_H
