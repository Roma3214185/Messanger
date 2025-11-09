#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QListView>

#include "dto/User.h"
#include "models/UserModel.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"

class IMainWindow {
 public:
  virtual void setUser(const User& user)                     = 0;
  virtual void setChatWindow(std::shared_ptr<ChatBase> chat) = 0;
  virtual void setChatModel(ChatModel* model)                = 0;
  virtual void setUserModel(UserModel* userModel)            = 0;
  virtual void clearFindUserEdit()                           = 0;
  virtual void showError(const QString& error)               = 0;
  virtual void setMessageListView(QListView* listView)       = 0;
  virtual void setCurrentChatIndex(QModelIndex idx)          = 0;
};

#endif  // IMAINWINDOW_H
