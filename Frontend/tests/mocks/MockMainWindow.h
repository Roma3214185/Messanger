#ifndef MOCKMAINWINDOW_H
#define MOCKMAINWINDOW_H

#include "interfaces/IMainWindow.h"

class MockMainWindow : public IMainWindow {
 public:
  void setChatWindow(std::shared_ptr<ChatBase> chat) override {
    ++call_setChatWindow;
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

  int call_setUser = 0;
  int call_setChatWindow = 0;
};

#endif // MOCKMAINWINDOW_H
