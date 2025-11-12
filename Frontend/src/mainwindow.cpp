#include "mainwindow.h"

#include <QFrame>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>

#include "../forms/ui_mainwindow.h"
#include "Debug_profiling.h"
#include "datainputservice.h"
#include "delegators/chatitemdelegate.h"
#include "delegators/messagedelegate.h"
#include "delegators/userdelegate.h"
#include "dto/SignUpRequest.h"
#include "interfaces/ITheme.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"
#include "presenter.h"
#include "presenter.h"

MainWindow::MainWindow(Model* model, QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , presenter_(std::make_unique<Presenter>(this, model)) {
  ui_->setupUi(this);

  presenter_->initialise();
  setDelegators();
  seupConnections();
  setupUI();
}

void MainWindow::setDelegators() {
  auto* chatDelegate = new ChatItemDelegate(this);
  auto* userDelegate = new UserDelegate(this);

  ui_->chatListView->setItemDelegate(chatDelegate);
  ui_->userListView->setItemDelegate(userDelegate);
}

void MainWindow::setChatModel(ChatModel* model) { ui_->chatListView->setModel(model); }

void MainWindow::setChatWindow(std::shared_ptr<ChatBase> chat) {
  ui_->messageWidget->setVisible(true);
  QString       name = chat->title;
  QPixmap       avatar(chat->avatar_path);
  constexpr int kAvatarSize    = 40;
  const QString kDefaultAvatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  if (!avatar.isNull()) {
    ui_->avatarTitle->setPixmap(
        avatar.scaled(kAvatarSize, kAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  } else {
    ui_->avatarTitle->setPixmap(QPixmap(kDefaultAvatar).scaled(kAvatarSize, kAvatarSize));
  }
  ui_->nameTitle->setText(name);
}

void MainWindow::setMessageListView(QListView* list_view) {
  ui_->messageListViewLayout->addWidget(list_view);

  auto* message_delegate = new MessageDelegate(this);
  list_view->setItemDelegate(message_delegate);
}

MainWindow::~MainWindow() { delete ui_; }

void MainWindow::on_upSubmitButton_clicked() {
  SignUpRequest signup_request;
  signup_request.email    = ui_->upEmail->text().trimmed();
  signup_request.password = ui_->upPassword->text().trimmed();
  signup_request.tag      = ui_->upTag->text().trimmed();
  signup_request.name     = ui_->upName->text().trimmed();

  auto res = DataInputService::validateRegistrationUserInput(signup_request);
  if (!res.valid)
    showError(res.message);
  else
    presenter_->signUp(signup_request);
}

void MainWindow::on_inSubmitButton_clicked() {
  LogInRequest login_request;
  login_request.email    = ui_->inEmail->text().trimmed();
  login_request.password = ui_->inPassword->text().trimmed();
  auto res               = DataInputService::validateLoginUserInput(login_request);
  if (!res.valid)
    showError(res.message);
  else
    presenter_->signIn(login_request);
}

void MainWindow::setMainWindow() {
  ui_->mainStackedWidget->setCurrentIndex(1);
  ui_->messageWidget->setVisible(false);
}

void MainWindow::setUser(const User& user) { setMainWindow(); }

void MainWindow::showError(const QString& error) { QMessageBox::warning(this, "ERROR", error); }

void MainWindow::setUserModel(UserModel* user_model) { ui_->userListView->setModel(user_model); }

void MainWindow::on_userTextEdit_textChanged(const QString& text) {
  presenter_->findUserRequest(text);
}

void MainWindow::on_textEdit_textChanged() {
  constexpr int kMinTextEditHeight = 200;
  constexpr int kAdditionalSpace   = 10;
  int           docHeight          = static_cast<int>(ui_->textEdit->document()->size().height());
  int           newHeight          = qMin(kMinTextEditHeight, docHeight + kAdditionalSpace);
  ui_->textEdit->setFixedHeight(newHeight);
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
}

void MainWindow::setSignInPage() {
  ui_->mainStackedWidget->setCurrentIndex(0);
  ui_->SignInUpWidget->setCurrentIndex(0);
  ui_->SignInButton->setEnabled(false);
  ui_->signUpButton->setEnabled(true);
  ui_->inEmail->clear();
  ui_->inPassword->clear();
}

void MainWindow::setSignUpPage() {
  ui_->mainStackedWidget->setCurrentIndex(0);
  ui_->SignInUpWidget->setCurrentIndex(1);
  ui_->SignInButton->setEnabled(true);
  ui_->signUpButton->setEnabled(false);
  clearUpInput();
}

void MainWindow::clearUpInput() {
  ui_->upEmail->clear();
  ui_->upName->clear();
  ui_->upPassword->clear();
  ui_->upTag->clear();
}

void MainWindow::seupConnections() {
  connect(ui_->chatListView, &QListView::clicked, this, [=](const QModelIndex& index) -> void {
    auto chat_id = index.data(ChatModel::ChatIdRole).toInt();
    presenter_->onChatClicked(chat_id);
  });

  connect(ui_->userListView, &QListView::clicked, this, [=](const QModelIndex& index) -> void {
    auto user_id = index.data(UserModel::UserIdRole).toInt();
    presenter_->onUserClicked(user_id);
  });

  connect(ui_->SignInButton, &QPushButton::clicked, this, &MainWindow::setSignInPage);
  connect(ui_->signUpButton, &QPushButton::clicked, this, &MainWindow::setSignUpPage);
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

void MainWindow::setCurrentChatIndex(QModelIndex chat_idx) {
  ui_->chatListView->setCurrentIndex(chat_idx);
}

void MainWindow::setTheme(std::unique_ptr<ITheme> theme) {
  currentTheme = std::move(theme);
  ui_->centralwidget->setStyleSheet(currentTheme->getStyleSheet());
}

void MainWindow::on_pushButton_clicked(bool checked) {
  if (checked) {
    setTheme(std::make_unique<DarkTheme>());
  } else {
    setTheme(std::make_unique<LightTheme>());
  }
}
