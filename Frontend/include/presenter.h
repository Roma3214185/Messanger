#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <optional>

#include "MessageListView.h"

class ChatModel;
class IMainWindow;
class Model;
class Message;
class SignUpRequest;
class UserModel;
class User;
class QJsonObject;

template <typename T>
using Optional    = std::optional<T>;
using OptionalId = std::optional<long long>;

class Presenter : public QObject {
  Q_OBJECT
 public:
  Presenter(IMainWindow* window, Model* manager);

  void signIn(const LogInRequest& login_request);
  void initialise();
  void setMessageListView(IMessageListView* message_list_view);
  void signUp(const SignUpRequest& req);
  void onChatClicked(const long long chat_id);
  void findUserRequest(const QString& text);
  void onUserClicked(const long long user_id, const bool is_user = true);
  void sendButtonClicked(const QString& text_to_send);
  void onLogOutButtonClicked();
  void onScroll(int value);
  //void setId(int id);

 protected:
  void setCurrentChatId(long long chat_id);
  void newMessage(Message& message);
  void onNewResponce(QJsonObject& json_object);

  OptionalId          current_opened_chat_id_;
  std::optional<User>  current_user_;

 private:
  void initialConnections();
  void setUser(const User& user, const QString& token);
  void openChat(long long chat_id);
  void onErrorOccurred(const QString& error);
  void onChatUpdated(long long chat_id);

  IMainWindow*         view_;
  Model*               manager_;
  IMessageListView*    message_list_view_;
};

#endif  // PRESENTER_H
