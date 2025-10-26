#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <optional>

#include "ChatModel/chatmodel.h"
#include "Model/model.h"
#include "UserModel/UserModel.h"
#include "headers/IMainWindow.h"
#include "headers/MessageListView.h"
#include "headers/SignUpRequest.h"

template <typename T>
using Optional = std::optional<T>;
using OptionalInt = Optional<int>;

class Presenter : public QObject {
  Q_OBJECT

 public:
  Presenter(IMainWindow* window, Model* manager);

  void signIn(const QString& email, const QString& password);
  void signUp(const SignUpRequest& req);
  void on_chat_clicked(const int chatId);
  void findUserRequest(const QString& text);
  void on_user_clicked(const int userId, const bool isUser = true);
  void sendButtonClicked(const QString& textToSend);
  void on_logOutButtonClicked();
  void onScroll(int value);
  void setId(int id);

 private:
  void initialConnections();
  void setUser(const User& user, const QString& token);
  void newMessage(Message& message);
  void openChat(const int chatId);
  void onErrorOccurred(const QString& error);
  void onChatUpdated(int chatId);

  IMainWindow* view_;
  Model* manager_;
  std::unique_ptr<MessageListView> messageListView;
  OptionalInt currentChatId_;
  OptionalInt currentUserId_;
};

#endif  // PRESENTER_H
