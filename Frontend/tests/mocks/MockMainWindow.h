#ifndef MOCKMAINWINDOW_H
#define MOCKMAINWINDOW_H

#include "interfaces/IMainWindow.h"

class MockMainWindow : public IMainWindow {
 public:
  void setUser(const User& user) override {

  }

  void setChatWindow(std::shared_ptr<ChatBase> chat) override {

  }

  void setChatModel(ChatModel* model) override {

  }

  void setUserModel(UserModel* userModel) override {

  }

  void clearFindUserEdit()  override {

  }

  void showError(const QString& error)  override {

  }

  void setMessageListView(QListView* listView) override {

  }

  void setCurrentChatIndex(QModelIndex idx) override {

  }
};

#endif // MOCKMAINWINDOW_H
