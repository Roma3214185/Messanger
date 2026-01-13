#include "mainwindow.h"

#include <QFrame>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QStandardItem>
#include <QTimer>
#include <QMouseEvent>

#include "../forms/ui_mainwindow.h"
#include "DataInputService.h"
#include "Debug_profiling.h"
#include "delegators/chatitemdelegate.h"
#include "delegators/messagedelegate.h"
#include "delegators/userdelegate.h"
#include "dto/SignUpRequest.h"
#include "interfaces/ITheme.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"
#include "presenter.h"

namespace MessageRoles {
enum { MessageIdRole = Qt::UserRole + 1, MessageTextRole };
}

MainWindow::MainWindow(Model *model, QWidget *parent)
    : QMainWindow(parent),
      ui_(std::make_unique<Ui::MainWindow>()),
      presenter_(std::make_unique<Presenter>(this, model)),
      searchResultsModel_(std::make_unique<QStandardItemModel>(this)),
      message_list_view_(std::make_unique<MessageListView>()) {
  ui_->setupUi(this);

  ui_->serch_messages_list_view->setModel(searchResultsModel_.get());
  ui_->serch_messages_list_view->sizeHintForRow(0);
  ui_->serch_messages_list_view->frameWidth();
  presenter_->setMessageListView(message_list_view_.get());
  presenter_->initialise();

  setDelegators();
  seupConnections();
  setupUI();
  setWriteMode();
}

void MainWindow::adjustSearchResultsHeight() {
  auto *view = ui_->serch_messages_list_view;
  auto *model = view->model();

  if (!model || model->rowCount() == 0) {
    view->setFixedHeight(0);
    return;
  }

  const int maxRows = 5;
  int rows = qMin(model->rowCount(), maxRows);

  int rowHeight = view->sizeHintForRow(0);
  int frame = view->frameWidth() * 2;

  int height = rowHeight * rows + frame;

  view->setFixedHeight(height);
}

void MainWindow::setDelegators() {
  auto *chat_delegate = presenter_->getChatDelegate();
  auto *user_delegate = presenter_->getUserDelegate();

  ui_->chatListView->setItemDelegate(chat_delegate);
  ui_->userListView->setItemDelegate(user_delegate);
}

void MainWindow::setChatModel(ChatModel *model) { ui_->chatListView->setModel(model); }

void MainWindow::setChatWindow(std::shared_ptr<ChatBase> chat) {
  DBC_REQUIRE(chat->chat_id > 0);
  DBC_REQUIRE(!chat->title.isEmpty());
  DBC_REQUIRE(!chat->avatar_path.isEmpty());
  ui_->messageWidget->setVisible(true);
  setWriteMode();
  setTitleChatMode();
  const QString name = chat->title;
  QPixmap avatar(chat->avatar_path);
  constexpr int kAvatarSize = 40;
  const QString kDefaultAvatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  if (!avatar.isNull()) {
    ui_->avatarTitle->setPixmap(avatar.scaled(kAvatarSize, kAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  } else {
    ui_->avatarTitle->setPixmap(QPixmap(kDefaultAvatar).scaled(kAvatarSize, kAvatarSize));
  }
  ui_->nameTitle->setText(name);
}

void MainWindow::setMessageListView(QListView *list_view) {
  DBC_REQUIRE(list_view != nullptr);
  ui_->messageListViewLayout->addWidget(list_view);

  message_delegate_ = presenter_->getMessageDelegate();
  list_view->setItemDelegate(message_delegate_);
}

MainWindow::~MainWindow() = default;

void MainWindow::on_upSubmitButton_clicked() {
  SignUpRequest signup_request;
  signup_request.email = ui_->upEmail->text().trimmed();
  signup_request.password = ui_->upPassword->text().trimmed();
  signup_request.tag = ui_->upTag->text().trimmed();
  signup_request.name = ui_->upName->text().trimmed();

  auto res = DataInputService::validateRegistrationUserInput(signup_request);
  if (!res.valid)
    showError(res.message);
  else
    presenter_->signUp(signup_request);
}

void MainWindow::on_inSubmitButton_clicked() {
  LogInRequest login_request;
  login_request.email = ui_->inEmail->text().trimmed();
  login_request.password = ui_->inPassword->text().trimmed();
  auto res = DataInputService::validateLoginUserInput(login_request);
  if (!res.valid) {
    showError(res.message);
  } else {
    presenter_->signIn(login_request);
  }
}

void MainWindow::setMainWindow() {
  ui_->mainStackedWidget->setCurrentIndex(1);
  ui_->messageWidget->setVisible(false);
}

void MainWindow::showError(const QString &error) {
  DBC_REQUIRE(!error.isEmpty());
  QMessageBox::warning(this, "ERROR", error);
}

void MainWindow::setUserModel(UserModel *user_model) {
  DBC_REQUIRE(user_model != nullptr);
  ui_->userListView->setModel(user_model);
}

void MainWindow::on_userTextEdit_textChanged(const QString &text) {
  if (!text.isEmpty()) presenter_->findUserRequest(text);
}

void MainWindow::on_textEdit_textChanged() {
  constexpr int kMinTextEditHeight = 200;
  constexpr int kAdditionalSpace = 10;
  const int docHeight = static_cast<int>(ui_->textEdit->document()->size().height());
  const int new_height = qMin(kMinTextEditHeight, docHeight + kAdditionalSpace);
  ui_->textEdit->setFixedHeight(new_height);
}

void MainWindow::on_sendButton_clicked() {
  auto text_to_send = ui_->textEdit->toPlainText();

  ui_->textEdit->clear();
  presenter_->sendButtonClicked(text_to_send);
}

void MainWindow::clearFindUserEdit() { ui_->userTextEdit->clear(); }

void MainWindow::on_logoutButton_clicked() {
  presenter_->onLogOutButtonClicked();
  setSignInPage();
  editable_message_.reset();
}

void MainWindow::setSignInPage() {
  ui_->mainStackedWidget->setCurrentIndex(0);
  ui_->SignInUpWidget->setCurrentIndex(0);
  ui_->SignInButton->setEnabled(false);
  ui_->signUpButton->setEnabled(true);
  ui_->inEmail->clear();
  ui_->inPassword->clear();
  editable_message_.reset();
}

void MainWindow::setSignUpPage() {
  ui_->mainStackedWidget->setCurrentIndex(0);
  ui_->SignInUpWidget->setCurrentIndex(1);
  ui_->SignInButton->setEnabled(true);
  ui_->signUpButton->setEnabled(false);
  clearUpInput();
  editable_message_.reset();
}

void MainWindow::clearUpInput() {
  ui_->upEmail->clear();
  ui_->upName->clear();
  ui_->upPassword->clear();
  ui_->upTag->clear();
}

void MainWindow::seupConnections() {
  connect(ui_->chatListView, &QListView::clicked, this, [this](const QModelIndex &index) -> void {
    long long chat_id = index.data(ChatModel::ChatIdRole).toLongLong();
    presenter_->onChatClicked(chat_id);
  });

  connect(ui_->userListView, &QListView::clicked, this, [this](const QModelIndex &index) -> void {
    long long user_id = index.data(UserModel::UserIdRole).toLongLong();
    presenter_->onUserClicked(user_id);
  });

  connect(ui_->SignInButton, &QPushButton::clicked, this, &MainWindow::setSignInPage);
  connect(ui_->signUpButton, &QPushButton::clicked, this, &MainWindow::setSignUpPage);

  DBC_REQUIRE(message_list_view_ != nullptr);
  message_list_view_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(message_list_view_.get(), &MessageListView::clickedWithEvent, this, &MainWindow::onPressEvent);

  connect(presenter_.get(), &Presenter::userSetted, this, &MainWindow::setMainWindow);

  // connect(searchResultsModel_, &QAbstractItemModel::rowsInserted,
  //         this, &MainWindow::adjustSearchResultsHeight);

  // connect(searchResultsModel_, &QAbstractItemModel::rowsRemoved,
  //         this, &MainWindow::adjustSearchResultsHeight);

  // connect(searchResultsModel_, &QAbstractItemModel::modelReset,
  //         this, &MainWindow::adjustSearchResultsHeight);
}

void MainWindow::setupUI() {
  ui_->pushButton->setCheckable(true);
  ui_->chatListView->setMouseTracking(true);
  constexpr int kHeightTextEdit = 35;
  setSignInPage();
  ui_->textEdit->setFixedHeight(kHeightTextEdit);
  ui_->textEdit->setPlaceholderText("Type a message...");

  ui_->chatListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_->userListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  ui_->userListView->setAttribute(Qt::WA_Hover, false);
  ui_->chatListView->setAttribute(Qt::WA_Hover, false);

  ui_->userListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui_->chatListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui_->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  ui_->messageWidget->setVisible(false);
  ui_->textEdit->setFrameStyle(QFrame::NoFrame);

  setTheme(std::make_unique<LightTheme>());
}

void MainWindow::setCurrentChatIndex(QModelIndex chat_idx) { ui_->chatListView->setCurrentIndex(chat_idx); }

void MainWindow::setTheme(std::unique_ptr<ITheme> theme) {
  DBC_REQUIRE(theme != nullptr);
  current_theme_ = std::move(theme);
  DBC_ENSURE(current_theme_ != nullptr);
  ui_->centralwidget->setStyleSheet(current_theme_->getStyleSheet());
}

void MainWindow::onPressEvent(QMouseEvent *event) {
  const QPoint pos = event->pos();
  qDebug() << "onPressEvent";

  if (event->button() == Qt::RightButton) {
    onMessageContextMenu(pos);
  } else if (event->button() == Qt::LeftButton) {
    onReactionClicked(pos);
  }

  //MessageListView::mousePressEvent(event);
}

void MainWindow::onReactionClicked(const QPoint &pos) {
  QModelIndex index = message_list_view_->indexAt(pos);  //todo: u get index 2 times, consider to refactor it in onPressEvent or in helper fucntion for future
  if (!index.isValid()) return;
  Message msg = index.data(MessageModel::Roles::FullMessage).value<Message>();
  if (auto reaction_id = message_delegate_->reactionAt(msg.id, pos)) {
    presenter_->reactionClicked(msg, *reaction_id);
  }
}

void MainWindow::on_pushButton_clicked(bool checked) {
  checked ? setTheme(std::make_unique<DarkTheme>()) : setTheme(std::make_unique<LightTheme>());
}

void MainWindow::onMessageContextMenu(const QPoint &pos) {
  QModelIndex index = message_list_view_->indexAt(pos);
  if (!index.isValid()) return;

  Message msg = index.data(MessageModel::Roles::FullMessage).value<Message>();

  QMenu menu(this);

  QAction *copyAction = menu.addAction("Copy");
  QAction *editAction = menu.addAction("Edit");
  QAction *deleteAction = menu.addAction("Delete");
  //todo: reactions = getStandart Reactions Menu, and make polymorhic

  QIcon like_icon("/Users/roma/QtProjects/Chat/images/like.jpeg");

  QAction *likeAction = menu.addAction(like_icon, "Like");
  likeAction->setIconVisibleInMenu(true);

  QIcon dislike_icon("/Users/roma/QtProjects/Chat/images/dislike.jpeg");
  QAction *dislikeAction = menu.addAction(dislike_icon, "Dislike");
  dislikeAction->setIconVisibleInMenu(true);


  if (msg.id <= 0 || !msg.isMine()) {  // message still offline, todo: add if it's your message
                                      // and if u are admin in this chat
    editAction->setEnabled(false);
    deleteAction->setEnabled(false);
    likeAction->setEnabled(false);
    dislikeAction->setEnabled(false);
  }

  if(msg.receiver_reaction == 1) likeAction->setEnabled(false);
  if(msg.receiver_reaction == 2) dislikeAction->setEnabled(false);

  QAction *selected = menu.exec(message_list_view_->viewport()->mapToGlobal(pos));
  if (!selected) return;

  if (selected == copyAction) {
    copyMessage(msg);
  } else if (selected == editAction) {
    editMessage(msg);
  } else if (selected == deleteAction) {
    deleteMessage(msg);
  } else if(selected == likeAction) {
    presenter_->reactionClicked(msg, 1);
  } else if(selected == dislikeAction) {
    presenter_->reactionClicked(msg, 2);
  }
}

void MainWindow::copyMessage(const Message &message) { qDebug() << "Copy " << message.toString(); }

void MainWindow::editMessage(const Message &message) {
  qDebug() << "Edit " << message.toString();
  DBC_REQUIRE(message.isMine() && !message.isOfflineSaved());

  QString text_to_edit = message.text;
  DBC_REQUIRE(!text_to_edit.isEmpty());
  if (text_to_edit.isEmpty()) return;

  ui_->inputEditStackedWidget->setCurrentIndex(1);
  ui_->editTextEdit->setText(text_to_edit);
  editable_message_ = message;
}

void MainWindow::deleteMessage(const Message &message) {
  qDebug() << "Delete " << message.toString();
  DBC_REQUIRE(message.isMine() && message.id > 0);
  presenter_->deleteMessage(message);
}

void MainWindow::on_cancelEditButton_clicked() {
  setWriteMode();
  editable_message_.reset();
}

void MainWindow::on_okEditButton_clicked() {
  QString current_text = ui_->editTextEdit->toPlainText();
  DBC_REQUIRE(!current_text.isEmpty());
  DBC_REQUIRE(editable_message_ != std::nullopt);
  setWriteMode();
  ui_->editTextEdit->clear();
  Message message_to_update = *editable_message_;
  message_to_update.text = current_text;
  presenter_->updateMessage(message_to_update);
}

void MainWindow::on_editTextEdit_textChanged() {
  QString current_text = ui_->editTextEdit->toPlainText();
  ui_->okEditButton->setEnabled(!current_text.isEmpty());
}

void MainWindow::setWriteMode() { ui_->inputEditStackedWidget->setCurrentIndex(0); }

QModelIndex MainWindow::findIndexByMessageId(QAbstractItemModel *model, long long id) {
  for (int row = 0; row < model->rowCount(); ++row) {
    QModelIndex idx = model->index(row, 0);
    if (idx.data(MessageModel::MessageIdRole).toLongLong() == id) return idx;
  }
  return {};
}

void MainWindow::setSearchMessageMode() {
  ui_->chat_title_stacked_widget->setCurrentIndex(1);
  ui_->serch_messages_list_view->setEnabled(false);
}

void MainWindow::setTitleChatMode() { ui_->chat_title_stacked_widget->setCurrentIndex(0); }

void MainWindow::on_serch_in_chat_button_clicked() {  // todo: remove this func
                                                      // and make just connect to
                                                      // setSearchMessageMode
  setSearchMessageMode();
}

void MainWindow::on_cancel_search_messages_button_clicked() {
  setTitleChatMode();
  ui_->search_messages_line_edit->clear();
  ui_->serch_messages_list_view->setEnabled(false);
  // todo: clear all
}

void MainWindow::on_search_messages_line_edit_textChanged(const QString &prefix) {
  // ui_->search_messages_line_edit->clear();
  searchResultsModel_->clear();
  if (prefix.isEmpty()) {
    ui_->serch_messages_list_view->setEnabled(false);
    return;
  }

  ui_->serch_messages_list_view->setEnabled(true);

  auto list_of_message = presenter_->getListOfMessagesBySearch(prefix);  // current_open_id is in presenter;

  if (list_of_message.empty()) {
    // resultsModel_.setTitle("No results");
    return;
  }

  for (Message &message : list_of_message) {
    QStandardItem *item = new QStandardItem(message.text);

    item->setData(message.id, MessageRoles::MessageIdRole);
    item->setData(message.text, MessageRoles::MessageTextRole);

    searchResultsModel_->appendRow(item);
  }
  adjustSearchResultsHeight();
}

void MainWindow::on_serch_messages_list_view_clicked(const QModelIndex &index) {
  if (!index.isValid()) return;

  long long messageId = index.data(MessageModel::MessageIdRole).toLongLong();

  qDebug() << "Clicked " << messageId;

  QModelIndex target = findIndexByMessageId(message_list_view_->model(), messageId);

  if (!target.isValid()) return;

  message_list_view_->scrollTo(target, QAbstractItemView::PositionAtCenter);

  message_list_view_->setCurrentIndex(target);

  // highlightMessage(messageId);
}
