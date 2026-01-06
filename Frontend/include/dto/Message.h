#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>
#include <QUuid>

#include "Debug_profiling.h"

struct Message { // todo: make immutable messagedomein and mutable messageview
  long long id = 0;
  long long sender_id;
  long long chat_id;
  QString text;
  QDateTime timestamp;
  bool readed_by_me;
  int read_counter = 0;
  int liked_counter = 0;
  bool status_sended{false};
  bool is_mine{false};
  std::optional<int> my_reaction;
  QString local_id;

  void updateFrom(const Message &other) {
    DBC_REQUIRE(local_id == other.local_id);
    LOG_INFO("Update from {}", other.toString());
    id = other.id;
    text = other.text;
    timestamp = other.timestamp;
    readed_by_me = other.readed_by_me;
    read_counter = other.read_counter;
    liked_counter = other.liked_counter;
    status_sended = other.status_sended;
    is_mine = other.is_mine;
    DBC_INVARIANT(checkInvariants());
  }

  std::string toString() const noexcept {
    std::string res;
    res += "| id = " + std::to_string(id);
    res += " | sender_id = " + std::to_string(sender_id);
    res += " | chat_id = " + std::to_string(chat_id);
    res += " | text = " + text.toStdString();
    res += " | timestamp = " + timestamp.toString().toStdString();
    res += " | readed_by_me = " + std::to_string(readed_by_me + 0);
    // res += " | liked_by_me = " +  std::to_string(liked_by_me + 0);
    res += " | read_counter = " + std::to_string(read_counter + 0);
    // res += " | liked_counter = " +  std::to_string(liked_counter + 0);
    res += " | status_sended = " + std::to_string(status_sended + 0);
    res += " | local_id = " + local_id.toStdString();
    res += " | is_mine = " + std::to_string(is_mine + 0);
    return res;
  }

  bool checkInvariants() const noexcept {
    return id > 0 // todo: what if message saved as offline, them id will == 0,
                  // state pattern??
           && sender_id > 0 && chat_id > 0 && !local_id.isEmpty() &&
           read_counter >= 0 && liked_counter >= 0 && !text.isEmpty();
  }
};

// todo:
//  struct MessageView {
//      Message message;       // immutable core
//      bool isRead = false;   // mutable UI state
//      bool isLiked = false;
//   const bool      readed_by_me;
//  const bool      liked_by_me;
//  const int       read_counter  = 0;
//  const int       liked_counter = 0;
//  const bool      status_sended = true;
//  };

#endif // MESSAGE_H
