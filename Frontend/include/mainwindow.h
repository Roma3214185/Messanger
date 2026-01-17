#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>
#include <QMainWindow>
#include <QStandardItemModel>
#include <memory>

#include "OutsideClickFilter.h"
#include "interfaces/IMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Presenter;
class ChatBase;
class QListView;
class ITheme;
class MessageListView;
class MessageDelegate;
class MessageModel;
class ClickOutsideClosableListView;

class MainWindow : public QMainWindow, public IMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(Model *model, QWidget *parent = nullptr);
  ~MainWindow();

  void setChatWindow(std::shared_ptr<ChatBase> chat) override;
  void setChatModel(ChatModel *model) override;
  void setUserModel(UserModel *user_model) override;
  void clearFindUserEdit() override;
  void showError(const QString &error) override;
  void setCurrentChatIndex(QModelIndex chat_idx) override;

  void setTheme(std::unique_ptr<ITheme> theme);

  bool eventFilter(QObject *, QEvent *event) override;

 Q_SIGNALS:
  void clickedOnPos(QPoint point);
  void geometryChanged();

 private Q_SLOTS:
  void on_upSubmitButton_clicked();
  void on_inSubmitButton_clicked();
  void on_userTextEdit_textChanged(const QString &text);
  void on_textEdit_textChanged();
  void on_sendButton_clicked();
  void on_logoutButton_clicked();
  void on_pushButton_clicked(bool checked);
  void onPressEvent(QMouseEvent *event);
  void on_cancelEditButton_clicked();
  void on_okEditButton_clicked();
  void on_editTextEdit_textChanged();
  void setWriteMode();
  void on_search_messages_line_edit_textChanged(const QString &arg1);
  void on_serch_messages_list_view_clicked(const QModelIndex &index);
  void closeEmojiMenu(bool close_manually = true);
  void onEmojiButtonStateChanged(bool open);

 private:
  void setMainWindow() const;
  void cancelSearchMessagesMode();
  void setSignInPage();
  void setMessageListView();
  void setSignUpPage();
  void clearUpInput();
  void setDelegators();
  void seupConnections();
  void setupUI();
  void openEmojiMenu();
  void onEmojiClicked(const ReactionInfo &emoji);

  void copyMessage(const Message &message);
  void editMessage(const Message &message);
  void deleteMessage(const Message &message);

  void setSearchMessageMode();
  void setTitleChatMode();
  QStandardItemModel *getEmojiModel();
  void setupUserListView();
  void setupSearchMessageListView();
  void setupEmojiMenu();

  void onMessageContextMenu(const QPoint &pos);
  void onReactionClicked(const QPoint &pos);
  QModelIndex findIndexByMessageId(QAbstractItemModel *model, long long id);

  std::unique_ptr<ITheme> current_theme_;
  Ui::MainWindow *ui;
  std::unique_ptr<Presenter> presenter_;
  std::unique_ptr<MessageListView> message_list_view_;
  MessageDelegate *message_delegate_;
  std::optional<Message> editable_message_;  // todo: make Page to set in currentPage, in which
                                             // Message will be
  std::unique_ptr<MessageModel> searchResultsModel_;

  ClickOutsideClosableListView *userListView_ = nullptr;
  ClickOutsideClosableListView *searchMessageListView_ = nullptr;
  ClickOutsideClosableListView *emoji_menu_ = nullptr;
};

#endif  // MAINWINDOW_H
