#include "usecases/socketusecase.h"

#include <QJsonParseError>
#include <QJsonObject>

#include "dto/Message.h"
#include "Debug_profiling.h"

SocketUseCase::SocketUseCase(SocketManager *socket_manager) : socket_manager_(socket_manager) {
  connect(socket_manager_, &SocketManager::newTextFromSocket, this, &SocketUseCase::onMessageReceived); //todo: onResponce from server in use cases or in managers (???)
}

void SocketUseCase::initSocket(long long user_id) { socket_manager_->initSocket(user_id); }

void SocketUseCase::onMessageReceived(const QString& msg) {
  PROFILE_SCOPE("Model::onMessageReceived");
  LOG_INFO("[onMessageReceived] Message received from user {}: ", msg.toStdString());

  QJsonParseError parseError;
  auto            doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    LOG_ERROR("[onMessageReceived] Failed JSON parse: '{}'",
              parseError.errorString().toStdString());
    Q_EMIT errorOccurred("Invalid JSON received: " + parseError.errorString());
    return;
  }

  auto json_responce = doc.object();
  Q_EMIT newResponce(json_responce);
}

void SocketUseCase::connectSocket() { socket_manager_->connectSocket(); }

void SocketUseCase::sendMessage(const Message& msg) {
  PROFILE_SCOPE("Model::sendMessage");

  if (msg.text.trimmed().isEmpty()) {
    LOG_WARN("Empty message skipped. chatId={}, senderId={}", msg.chatId, msg.senderId);
    return;
  }

  auto json = QJsonObject{{"type", "send_message"},
                          {"sender_id", msg.senderId},
                          {"chat_id", msg.chatId},
                          {"text", msg.text},
                          {"timestamp", msg.timestamp.toString()},
                          {"local_id", msg.local_id}};

  socket_manager_->sendText(QString(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact))));
  LOG_INFO("[sendMessage] To send message to chatId={} from user {}: '{}'",
           msg.chatId, msg.senderId, msg.text.toStdString());
}
