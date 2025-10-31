#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>
#include <QMainWindow>

#include "headers/IMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Presenter;

class MainWindow : public QMainWindow, public IMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void setPresenter(Presenter* presenter);
  void setUser(const User& user) override;
  void setChatWindow(std::shared_ptr<ChatBase> chat) override;
  void setChatModel(ChatModel* model) override;
  void setUserModel(UserModel* user_model) override;
  void clearFindUserEdit() override;
  void showError(const QString& error) override;

  void setMessageListView(QListView* list_view) override;
  void setCurrentChatIndex(QModelIndex chat_idx) override;

 private Q_SLOTS:
  void on_upSubmitButton_clicked();
  void on_inSubmitButton_clicked();
  void on_userTextEdit_textChanged(const QString& text);
  void on_textEdit_textChanged();
  void on_sendButton_clicked();
  void on_logoutButton_clicked();

 private:
  void setMainWindow();
  void setSignInPage();
  void setSignUpPage();
  void clearUpInput();
  void setDelegators();
  void seupConnections();
  void setupUI();

  Ui::MainWindow* ui_;
  Presenter* presenter_;
};

#endif  // MAINWINDOW_H
