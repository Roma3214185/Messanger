#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <optional>

#include "headers/MessageListView.h"

class ChatModel;
class IMainWindow;
class Model;
class Message;
class SignUpRequest;
class UserModel;
class User;
class QJsonObject;

template <typename T>
using Optional = std::optional<T>;
using OptionalInt = Optional<int>;

class Presenter : public QObject {
  Q_OBJECT
 public:
  Presenter(IMainWindow* window, Model* manager);

  void signIn(const LogInRequest& login_request);
  void signUp(const SignUpRequest& req);
  void onChatClicked(const int chat_id);
  void findUserRequest(const QString& text);
  void onUserClicked(const int user_id, const bool is_user = true);
  void sendButtonClicked(const QString& text_to_send);
  void onLogOutButtonClicked();
  void onScroll(int value);
  void setId(int id);

 private:
  void initialConnections();
  void setUser(const User& user, const QString& token);
  void newMessage(Message& message);
  void openChat(int chat_id);
  void onErrorOccurred(const QString& error);
  void onChatUpdated(int chat_id);
  void onNewResponce(QJsonObject& json_object);

  IMainWindow* view_;
  Model* manager_;
  std::unique_ptr<MessageListView> message_list_view_;
  OptionalInt current_chat_id_;
  OptionalInt current_user_id_;
};

#endif  // PRESENTER_H
