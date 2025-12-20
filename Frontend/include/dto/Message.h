#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>
#include <QUuid>

struct Message {
  long long       id = 0;
  long long       senderId;
  long long       chatId;
  QString   text;
  QDateTime timestamp;
  bool      readed_by_me;
  bool      liked_by_me;
  int       read_counter  = 0;
  int       liked_counter = 0;
  bool      status_sended = true;
  QString   local_id;
};

#endif  // MESSAGE_H
