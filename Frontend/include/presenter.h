#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <optional>

#include "MessageListView.h"
#include "delegators/chatitemdelegate.h"
#include "delegators/messagedelegate.h"
#include "delegators/userdelegate.h"  //todo(roma): make forward declarations
#include "interfaces/ISocketResponceHandler.h"

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
using OptionalId = std::optional<long long>;

class Presenter : public QObject {
  Q_OBJECT
 public:
  using SocketHandlersMap = std::unordered_map<std::string, std::unique_ptr<ISocketResponceHandler>>;

  Presenter(IMainWindow *window, Model *manager);

  void signIn(const LogInRequest &login_request);
  void initialise();
  void setMessageListView(IMessageListView *message_list_view);
  void signUp(const SignUpRequest &req);
  void onChatClicked(const long long chat_id);
  void findUserRequest(const QString &text);
  void onUserClicked(const long long user_id, const bool is_user = true);
  void sendButtonClicked(const QString &text_to_send);
  void onLogOutButtonClicked();
  void onScroll(int value);

  void deleteMessage(const Message &message);
  void updateMessage(Message &message);
  void reactionClicked(Message &message, int reaction_id);

  MessageDelegate *getMessageDelegate();
  UserDelegate *getUserDelegate();
  ChatItemDelegate *getChatDelegate();

  std::vector<Message> getListOfMessagesBySearch(const QString &prefix);
  std::vector<ReactionInfo> getDefaultReactionsInChat(long long chat_id);

 Q_SIGNALS:
  void userSetted();

 protected:
  void setCurrentChatId(long long chat_id);
  void newMessage(Message &message);
  void onNewResponce(QJsonObject &json_object);
  void onUnreadMessage(Message &message);

  OptionalId current_opened_chat_id_;
  std::optional<User> current_user_;

 private:
  void initialConnections();
  void setUser(const User &user, const QString &token);
  void openChat(long long chat_id);
  void onErrorOccurred(const QString &error);
  void initialHandlers();

  SocketHandlersMap socket_responce_handlers_;
  IMainWindow *view_;
  Model *manager_;
  IMessageListView *message_list_view_;

  std::unique_ptr<MessageDelegate> message_delegate_;
  std::unique_ptr<UserDelegate> user_delegate_;
  std::unique_ptr<ChatItemDelegate> chat_delegate_;
};

#endif  // PRESENTER_H
