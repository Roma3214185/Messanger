#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>
#include <QMainWindow>
#include <memory>

#include "interfaces/IMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Presenter;
class ChatBase;
class User;
class QListView;
class ITheme;
class MessageListView;

class MainWindow : public QMainWindow, public IMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(Model* model, QWidget* parent = nullptr);
  ~MainWindow();

  void setChatWindow(std::shared_ptr<ChatBase> chat) override;
  void setChatModel(ChatModel* model) override;
  void setUserModel(UserModel* user_model) override;
  void clearFindUserEdit() override;
  void showError(const QString& error) override;

  void setMessageListView(QListView* list_view) override;
  void setCurrentChatIndex(QModelIndex chat_idx) override;

  void setTheme(std::unique_ptr<ITheme> theme);

 private Q_SLOTS:
  void on_upSubmitButton_clicked();
  void on_inSubmitButton_clicked();
  void on_userTextEdit_textChanged(const QString& text);
  void on_textEdit_textChanged();
  void on_sendButton_clicked();
  void on_logoutButton_clicked();
  void on_pushButton_clicked(bool checked);

  void on_cancelEditButton_clicked();

  void on_okEditButton_clicked();

  void on_editTextEdit_textChanged();

  void setWriteMode();
  //void setEditMode();

private:
  void setMainWindow();
  void setSignInPage();
  void setSignUpPage();
  void clearUpInput();
  void setDelegators();
  void seupConnections();
  void setupUI();

  void copyMessage(const Message& message);
  void editMessage(const Message& message);
  void deleteMessage(const Message& message);

  void onMessageContextMenu(const QPoint& pos);

  std::unique_ptr<ITheme>    current_theme_;
  Ui::MainWindow*            ui_;
  std::unique_ptr<Presenter> presenter_;
  std::unique_ptr<MessageListView> message_list_view_;

  std::optional<Message> editable_message_; //todo: make Page to set in currentPage, in which Message will be
};

#endif  // MAINWINDOW_H
