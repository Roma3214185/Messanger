#include "usecases/socketusecase.h"

#include <QJsonObject>
#include <QJsonParseError>

#include "Debug_profiling.h"
#include "dto/Message.h"
#include "entities/MessageStatus.h"
#include "utils.h"

SocketUseCase::SocketUseCase(std::unique_ptr<SocketManager> socket_manager)
    : socket_manager_(std::move(socket_manager)) {
  connect(socket_manager_.get(), &SocketManager::newTextFromSocket, this,
          &SocketUseCase::onMessageReceived);  // todo: onResponce from server in
                                               // use cases or in managers (???)
}

void SocketUseCase::initSocket(long long user_id) { socket_manager_->initSocket(user_id); }

void SocketUseCase::onMessageReceived(const QString& msg) {
  PROFILE_SCOPE("Model::onMessageReceived");
  LOG_INFO("[onMessageReceived] Message received from user {}: ", msg.toStdString());

  QJsonParseError parseError;
  auto doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    LOG_ERROR("[onMessageReceived] Failed JSON parse: '{}'", parseError.errorString().toStdString());
    Q_EMIT errorOccurred("Invalid JSON received: " + parseError.errorString());
    return;
  }

  auto json_responce = doc.object();
  Q_EMIT newResponce(json_responce);
}

void SocketUseCase::connectSocket() { socket_manager_->connectSocket(); }

void SocketUseCase::saveReaction(const Reaction& reaction) {
  QJsonObject json;
  json["type"] = "save_reaction";
  json["message_id"] = reaction.message_id;
  json["receiver_id"] = reaction.receiver_id;
  json["reaction_id"] = reaction.reaction_id;
  sendInSocket(json);
}

void SocketUseCase::deleteReaction(const Reaction& reaction) {
  QJsonObject json;
  json["type"] = "delete_reaction";
  json["message_id"] = reaction.message_id;  // todo: save and delete Reaction almost the same, differs only type
  json["receiver_id"] = reaction.receiver_id;
  json["reaction_id"] = reaction.reaction_id;
  sendInSocket(json);
}

void SocketUseCase::sendMessage(const Message& msg) {
  PROFILE_SCOPE("Model::sendMessage");

  if (msg.tokens.empty()) {
    LOG_WARN("Empty message skipped. chatId={}, senderId={}", msg.chat_id, msg.sender_id);
    return;
  }

  auto json = QJsonObject{{"type", "send_message"},
                          {"sender_id", msg.sender_id},
                          {"chat_id", msg.chat_id},
                          {"text", msg.getFullText()},
                          {"timestamp", msg.timestamp.toString()},
                          {"local_id", msg.local_id}};

  if (msg.answer_on.has_value()) {
    json["answer_on"] = msg.answer_on.value();  // todo: form Json In JsonService
  }

  LOG_INFO("[sendMessage] To send message to chatId={} from user {}: '{}'", msg.chat_id, msg.sender_id,
           msg.getFullText().toStdString());
  sendInSocket(json);
}

void SocketUseCase::sendReadMessageEvent(const MessageStatus& message_status) {
  DBC_REQUIRE(message_status.is_read);
  auto json = QJsonObject{
      {"type", "read_message"}, {"message_id", message_status.message_id}, {"readed_by", message_status.receiver_id}};
  // todo: readed_by -> receiver_id

  sendInSocket(json);
  // socket_manager_->sendText(QString(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact))));
}
void SocketUseCase::sendInSocket(const QString& text) { socket_manager_->sendText(text); }

void SocketUseCase::sendInSocket(const QJsonObject& json) {
  sendInSocket(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
}
