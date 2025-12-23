#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>
#include <QUuid>

#include "Debug_profiling.h"

struct Message { //todo: make immutable messagedomein and mutable messageview
  long long       id = 0;
  long long       senderId;
  long long       chatId;
  QString   text;
  QDateTime timestamp;
  bool      readed_by_me;
  bool      liked_by_me;
  int       read_counter  = 0;
  int       liked_counter = 0;
  bool      status_sended = false;
  QString   local_id;

  void updateFrom(const Message& other) {
    if(local_id != other.local_id) {
      LOG_ERROR("Can't updateFrom {} because it's local_ids differs");
      return;
    }

    if(senderId != other.senderId) {
      LOG_ERROR("Can't updateFrom {} because it's senderId differs");

      return;
    }

    if(chatId != other.chatId) {
      LOG_ERROR("Can't updateFrom {} because it's chatId differs");
      return;
    }

    id = other.id;
    text = other.text;
    timestamp = other.timestamp;
    readed_by_me = other.readed_by_me;
    liked_by_me = other.liked_by_me;
    read_counter = other.read_counter;
    liked_counter = other.liked_counter;
    status_sended = other.status_sended;
  }

  std::string to_string() {
    std::string res;
    res += "|id = " + id;
    res += "|senderId = " + senderId;
    res += "|chatId = " + chatId;
    res += "|text = " + text.toStdString();
    res += "|timestamp = " + timestamp.toString().toStdString();
    //res += "|readed_by_me = " + readed_by_me;
    //res += "|liked_by_me = " + liked_by_me;
    //res += "|read_counter = " + read_counter;
    //res += "|liked_counter = " + liked_counter;
    res += "|status_sended = " + status_sended;
    res += "|local_id = " + local_id.toStdString();
    return res;
  }
};

//todo:
// struct MessageView {
//     Message message;       // immutable core
//     bool isRead = false;   // mutable UI state
//     bool isLiked = false;
//  const bool      readed_by_me;
// const bool      liked_by_me;
// const int       read_counter  = 0;
// const int       liked_counter = 0;
// const bool      status_sended = true;
// };

#endif  // MESSAGE_H
