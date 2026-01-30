#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>
#include <QUuid>
#include <unordered_map>

#include "Debug_profiling.h"
#include "MessageToken.h"
#include "entities/Reaction.h"
#include "entities/ReactionInfo.h"
#include "utils.h"

struct Message {  // todo: make immutable messagedomein and mutable messageview
  long long id = 0;
  long long sender_id;
  long long chat_id;
  std::vector<MessageToken> tokens;
  QDateTime timestamp;
  bool receiver_read_status = false;
  int read_counter = 0;
  bool status_sended{false};
  long long receiver_id;
  std::optional<long long> receiver_reaction{std::nullopt};
  std::unordered_map<long long, int> reactions;
  QString local_id;
  std::optional<long long> answer_on;

  void updateFrom(const Message& other) {  // todo: copy asign operator
    LOG_INFO("Update");
    LOG_INFO("Update {}", this->toString());
    LOG_INFO("From {}", other.toString());

    DBC_REQUIRE(local_id == other.local_id);
    DBC_REQUIRE(chat_id == other.chat_id);
    DBC_REQUIRE(sender_id == other.sender_id);
    DBC_REQUIRE(receiver_id == other.receiver_id);
    if (id != 0) DBC_REQUIRE(id == other.id);

    id = other.id;
    tokens = other.tokens;
    timestamp = other.timestamp;
    receiver_read_status = other.receiver_read_status;
    read_counter = other.read_counter;
    reactions = other.reactions;
    status_sended = other.status_sended;
    receiver_reaction = other.receiver_reaction;
    reactions = other.reactions;
    answer_on = other.answer_on;
    DBC_INVARIANT(checkInvariants());
  }

  inline bool operator==(const Message& other) const {
    return local_id == other.local_id && chat_id == other.chat_id && sender_id == other.sender_id &&
           receiver_id == other.receiver_id && id == other.id && tokens == other.tokens &&
           timestamp == other.timestamp && receiver_read_status == other.receiver_read_status &&
           read_counter == other.read_counter && status_sended == other.status_sended &&
           receiver_reaction == other.receiver_reaction && reactions == other.reactions;
  }

  bool isOfflineSaved() const noexcept {
    return id == 0;  // todo: state pattern
  }

  bool isMine() const noexcept { return sender_id == receiver_id; }

  std::string toString() const {
    std::string res;
    res += "| id = " + std::to_string(id);
    res += " | sender_id = " + std::to_string(sender_id);
    res += " | chat_id = " + std::to_string(chat_id);
    res += " | text = " + getFullText().toStdString();
    res += " | timestamp = " + timestamp.toString().toStdString();
    res += " | receiver_read_status = " + std::to_string(receiver_read_status + 0);

    if (receiver_reaction.has_value()) {
      res += " | my reaction id is = " + std::to_string(*receiver_reaction);
    } else {
      res += " | no my reaction ";
    }

    if (answer_on.has_value()) {
      res += " | answer_on = " + std::to_string(answer_on.value());
    } else {
      res += " | no answer_on ";
    }

    res += " | read_counter = " + std::to_string(read_counter + 0);
    res += " | status_sended = " + std::to_string(status_sended + 0);
    res += " | local_id = " + local_id.toStdString();

    int total_reactions = 0;
    std::string reactions_string;
    for (const auto& [react_id, react_cnt] : reactions) {
      total_reactions += react_cnt;
      reactions_string += "| {reaction:" + std::to_string(react_id) + "; cnt: " + std::to_string(react_cnt) + "} ";
    }
    res += " | total_reactions = " + std::to_string(total_reactions);
    res += reactions_string;
    return res;
  }

  QString getPlainText() const {
    QString res;
    for (const auto& token : tokens) {
      if (token.type == MessageTokenType::Text) res += token.getText();
    }
    return res;
  }

  QString getFullText() const { return utils::text::tokenize(tokens); }

  bool checkInvariants() const {
    return id > 0 && sender_id > 0 && chat_id > 0 && !local_id.isEmpty() && read_counter >= 0 && receiver_id > 0 &&
           !tokens.empty() && (!receiver_reaction.has_value() || receiver_reaction.value() > 0) &&
           (!answer_on.has_value() || answer_on.value() > 0);
  }
};

Q_DECLARE_METATYPE(Message)

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

#endif  // MESSAGE_H
