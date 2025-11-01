#include "mainwindow.h"

#include <QFrame>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>

#include "ChatItemDelegate/chatitemdelegate.h"
#include "ChatModel/chatmodel.h"
#include "DataInputService/datainputservice.h"
#include "Debug_profiling.h"
#include "MessageDelegate/messagedelegate.h"
#include "MessageModel/messagemodel.h"
#include "Presenter/presenter.h"
#include "UserDelegate/userdelegate.h"
#include "headers/SignUpRequest.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui_(new Ui::MainWindow) {
  ui_->setupUi(this);

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

void MainWindow::setChatModel(ChatModel* model) {
  ui_->chatListView->setModel(model);
}

void MainWindow::setChatWindow(std::shared_ptr<ChatBase> chat) {
  ui_->messageWidget->setVisible(true);
  QString name = chat->title;
  QPixmap avatar(chat->avatar_path);
  constexpr int kAvatarSize = 40;
  const QString kDefaultAvatar =
      "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  if (!avatar.isNull()) {
    ui_->avatarTitle->setPixmap(avatar.scaled(kAvatarSize, kAvatarSize,
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));
  } else {
    ui_->avatarTitle->setPixmap(
        QPixmap(kDefaultAvatar).scaled(kAvatarSize, kAvatarSize));
  }
  ui_->nameTitle->setText(name);
}

void MainWindow::setMessageListView(QListView* list_view) {
  ui_->messageListViewLayout->addWidget(list_view);

  auto* message_delegate = new MessageDelegate(this);
  list_view->setItemDelegate(message_delegate);
}

MainWindow::~MainWindow() { delete ui_; }

void MainWindow::setPresenter(Presenter* presenter) {
  this->presenter_ = presenter;
}

void MainWindow::on_upSubmitButton_clicked() {
  auto email = ui_->upEmail->text().trimmed();
  auto password = ui_->upPassword->text().trimmed();
  auto tag = ui_->upTag->text().trimmed();
  auto name = ui_->upName->text().trimmed();

  if (!DataInputService::emailValid(email)) {
    showError("Email is invalid");
    return;
  }

  if (!DataInputService::passwordValid(password)) {
    showError("Password is invalid");
    return;
  }

  if (!DataInputService::tagValid(tag)) {
    showError("Tag is invalid");
    return;
  }

  if (!DataInputService::nameValid(name)) {
    showError("Name is invalid");
    return;
  }

  SignUpRequest request{
      .email = email, .password = password, .tag = tag, .name = name};

  presenter_->signUp(request);
}

void MainWindow::on_inSubmitButton_clicked() {
  auto email = ui_->inEmail->text().trimmed();
  auto password = ui_->inPassword->text().trimmed();

  if (!DataInputService::emailValid(email)) {
    showError("Email is invalid");
    return;
  }

  if (!DataInputService::passwordValid(password)) {
    showError("Password is invalid");
    return;
  }

  presenter_->signIn(email, password);
}

void MainWindow::setMainWindow() {
  ui_->mainStackedWidget->setCurrentIndex(1);
  ui_->messageWidget->setVisible(false);
}

void MainWindow::setUser(const User& user) { setMainWindow(); }

void MainWindow::showError(const QString& error) {
  QMessageBox::warning(this, "ERROR", error);
}

void MainWindow::setUserModel(UserModel* user_model) {
  ui_->userListView->setModel(user_model);
}

void MainWindow::on_userTextEdit_textChanged(const QString& text) {
  presenter_->findUserRequest(text);
}

void MainWindow::on_textEdit_textChanged() {
  constexpr int kMinTextEditHeight = 200;
  constexpr int kAdditionalSpace = 10;
  int docHeight = static_cast<int>(ui_->textEdit->document()->size().height());
  int newHeight = qMin(kMinTextEditHeight, docHeight + kAdditionalSpace);
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
  connect(ui_->chatListView, &QListView::clicked, this,
          [=](const QModelIndex& index) -> void {
            auto chat_id = index.data(ChatModel::ChatIdRole).toInt();
            presenter_->onChatClicked(chat_id);
          });

  connect(ui_->userListView, &QListView::clicked, this,
          [=](const QModelIndex& index) -> void {
            auto user_id = index.data(UserModel::UserIdRole).toInt();
            presenter_->onUserClicked(user_id);
          });

  connect(ui_->SignInButton, &QPushButton::clicked, this,
          &MainWindow::setSignInPage);
  connect(ui_->signUpButton, &QPushButton::clicked, this,
          &MainWindow::setSignUpPage);
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

  setLightTheme();
  // setDarkTheme();
}

void MainWindow::setCurrentChatIndex(QModelIndex chat_idx) {
  ui_->chatListView->setCurrentIndex(chat_idx);
}

void MainWindow::setLightTheme() {
  ui_->centralwidget->setStyleSheet(R"(
        QWidget {
            background-color: #eaf4ff;
            color: #1b1b1b;
            font-family: "Inter", "Arial";
        }

        QListView {
            background-color: #f5faff;
            border: none;
            outline: 0;
            selection-background-color: #b3d4ff;
            selection-color: #1b1b1b;
            show-decoration-selected: 0;
        }

        QListView::item {
            color: #1b1b1b;
            padding: 6px 8px;
        }

        QListView::item:selected {
            background-color: #b3d4ff;
            color: #1b1b1b !important;
        }

        QListView::item:hover {
            background: transparent;
            color: #1b1b1b;
        }

        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #c7dfff;
            border-radius: 8px;
            padding: 6px 10px;
            color: #1b1b1b;
        }

        QTextEdit {
            background-color: #ffffff;
            border: 1px solid #c7dfff;
            color: #1b1b1b;
        }

        QPushButton {
            background-color: #d0e7ff;
            border: 1px solid #b3d4ff;
            border-radius: 4px;
            padding: 4px 8px;
            color: #1b1b1b;
        }

        QPushButton:hover, QPushButton:pressed, QPushButton:checked {
            background-color: #d0e7ff;
            border: 1px solid #b3d4ff;
            color: #1b1b1b;
        }

        QLabel {
            background: transparent;
            color: #1b1b1b;
        }

        QScrollBar:vertical {
            background: #f5faff;
            width: 8px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical {
            background: #c2dcff;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: #b3d4ff;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            background: none;
            height: 0px;
        }
    )");
}

void MainWindow::setDarkTheme() {
  ui_->centralwidget->setStyleSheet(R"(
        QWidget {
            background-color: #0f1c2b;
            color: #e0e6f0;
            font-family: "Inter", "Arial";
        }

        QListView {
            background-color: #142133;
            border: 1px solid #0f2a45;
            outline: 0;
            selection-background-color: #1f4a7a;
            selection-color: #ffffff;
            show-decoration-selected: 0;
        }

        QListView::item {
            color: #e0e6f0;
            padding: 6px 8px;
        }


        QListView::item:selected,
        QListView::item:selected:focus,
        QListView::item:selected:!focus {
            background-color: #1f4a7a;
            color: #ffffff !important;
        }

        QListView::item:hover {
            background: transparent;
            color: #e0e6f0;
        }

        QPushButton {
            background-color: #22385b;
            border: 1px solid #3a547d;
            color: #e0e6f0;
            padding: 4px 8px;
            border-radius: 4px;
        }

        QPushButton:hover, QPushButton:pressed, QPushButton:checked {
            background-color: #22385b;
            border: 1px solid #3a547d;
            color: #e0e6f0;
        }

        QLabel {
            background: transparent;
            color: #e0e6f0;
        }

        QScrollBar:vertical {
            background: #142133;
            width: 8px;
            border-radius: 4px;
        }

        QTextEdit {
            background-color: #142133;
            border: 1px solid #0f2a45;
            color: #e0e6f0;
        }

        QLineEdit {
            background-color: #142133;
            border: 1px solid #0f2a45;
            color: #e0e6f0;
        }

        QScrollBar::handle:vertical {
            background: #2a3b55;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: #3a547d;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            background: none;
            height: 0px;
        }
    )");
}

void MainWindow::on_pushButton_clicked(bool checked) {
  if (checked)
    setDarkTheme();
  else
    setLightTheme();
}
