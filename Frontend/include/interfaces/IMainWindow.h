#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QModelIndex>
#include <QString>

#include "dto/ChatBase.h"
#include "models/chatmodel.h"

class UserModel;
class ChatBase;

class IMainWindow {
 public:
  virtual void setChatWindow(std::shared_ptr<ChatBase> chat) = 0;
  virtual void setChatModel(ChatModel *model) = 0;
  virtual void setUserModel(UserModel *user_model) = 0;
  virtual void showError(const QString &error) = 0;
  virtual void setCurrentChatIndex(QModelIndex idx) = 0;
};

#endif  // IMAINWINDOW_H
