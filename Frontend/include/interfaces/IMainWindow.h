#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QListView>

#include "dto/User.h"
#include "models/UserModel.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"

class IMainWindow {
 public:
  virtual void setChatWindow(std::shared_ptr<ChatBase> chat) = 0;
  virtual void setChatModel(ChatModel *model) = 0;
  virtual void setUserModel(UserModel *user_model) = 0;
  virtual void clearFindUserEdit() = 0;
  virtual void showError(const QString &error) = 0;
  virtual void setMessageListView(QListView *list_view) = 0;
  virtual void setCurrentChatIndex(QModelIndex idx) = 0;
};

#endif  // IMAINWINDOW_H
