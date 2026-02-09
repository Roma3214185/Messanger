#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <optional>

#include "dto/User.h"
#include "interfaces/ISocketResponceHandler.h"

class ChatModel;
class IMainWindow;
class Model;
class Message;
class SignUpRequest;
class UserModel;
class User;
class QJsonObject;
class QTextDocument;
class ISocketResponceHandler;
class MessageListView;
class ReactionInfo;
class LogInRequest;
class User;
class Reaction;

using OptionalId = std::optional<long long>;

class Presenter : public QObject {
  Q_OBJECT
 public:
  using SocketHandlersMap = std::unordered_map<std::string, std::unique_ptr<ISocketResponceHandler>>;
  Presenter(IMainWindow *window, Model *manager);

  void signIn(const LogInRequest &login_request);
  void initialise();
  void setMessageListView(MessageListView *message_list_view);
  void signUp(const SignUpRequest &signup_request);
  void onChatClicked(const long long chat_id);
  void findUserRequest(const QString &text);
  void onUserClicked(const long long user_id, const bool is_user = true);
  void sendButtonClicked(QTextDocument *document_with_text_to_send, OptionalId answer_on_message_id);
  void onLogOutButtonClicked();
  void onUnreadMessage(Message &message);
  void editMessage(Message &message_to_edit, QTextDocument *edited_text);

  void deleteMessage(const Message &message);
  void reactionClicked(const Message &message, long long reaction_id);

  std::vector<Message> getListOfMessagesBySearch(const QString &prefix);
  std::vector<ReactionInfo> getDefaultReactionsInChat(long long chat_id);
  std::vector<ReactionInfo> getReactionsForMenu();
  std::optional<ReactionInfo> getReactionInfo(long long reaction_id);
  void initialHandlers(SocketHandlersMap handlers);

 Q_SIGNALS:
  void userSetted();

 protected:
  void setCurrentChatId(long long chat_id);
  void onMessageReceivedFromSocket(const Message &message);
  void onNewSocketMessage(const QJsonObject &socket_message);

  OptionalId current_opened_chat_id_;
  std::optional<User> current_user_;

 private:
  void initialConnections();
  void setUser(const User &user, const QString &token);
  void openChat(long long chat_id);
  void onErrorOccurred(const QString &error);
  void saveReaction(const Reaction &reaction);
  void deleteReaction(const Reaction &reaction);
  void showError(const QString &error);
  void onChatWidgetScroll(int distance_from_top);

  SocketHandlersMap socket_responce_handlers_;
  IMainWindow *view_;
  Model *manager_;
  MessageListView *message_list_view_{nullptr};
};

#endif  // PRESENTER_H
