#include "mainwindow.h"

#include <QFrame>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStandardItem>
#include <QTimer>
#include <cstdint>

#include "../forms/ui_mainwindow.h"
#include "DataInputService.h"
#include "Debug_profiling.h"
#include "MessageActionPanel.h"
#include "delegators/chatitemdelegate.h"
#include "delegators/messagedelegate.h"
#include "delegators/userdelegate.h"
#include "dto/SignUpRequest.h"
#include "interfaces/ITheme.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"
#include "presenter.h"
#include "MessageActionPanel.h"
#include "utilsui.h"
#include "clickoutsideclosablelistview.h"

namespace MessageRoles {
enum { MessageIdRole = Qt::UserRole + 1, MessageTextRole };
}

class PopedAutoClosedList : QListView {
    QAbstractItemModel* model_;
  public:
    PopedAutoClosedList(QAbstractItemModel* model, QWidget* parent = nullptr) : model_(model), QListView(parent) {

    }
};

MainWindow::MainWindow(Model *model, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      presenter_(std::make_unique<Presenter>(this, model)),
      searchResultsModel_(std::make_unique<QStandardItemModel>(this)),
      message_list_view_(std::make_unique<MessageListView>()) {
  ui->setupUi(this);
  qApp->installEventFilter(this);

  presenter_->setMessageListView(message_list_view_.get());
  presenter_->initialise();

  setDelegators();
  seupConnections();
  setupUI();
  setWriteMode();
}

void MainWindow::setDelegators() {
  auto *chat_delegate = presenter_->getChatDelegate();
  ui->chatListView->setItemDelegate(chat_delegate);
}

void MainWindow::setChatModel(ChatModel *model) { ui->chatListView->setModel(model); }

void MainWindow::setChatWindow(std::shared_ptr<ChatBase> chat) {
  DBC_REQUIRE(chat->chat_id > 0);
  DBC_REQUIRE(!chat->title.isEmpty());
  DBC_REQUIRE(!chat->avatar_path.isEmpty());
  ui->messageWidget->setVisible(true);
  setWriteMode();
  setTitleChatMode();
  const QString name = chat->title;
  QPixmap avatar(chat->avatar_path);
  constexpr int kAvatarSize = 40;
  const QString kDefaultAvatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  if (!avatar.isNull()) {
    ui->avatarTitle->setPixmap(avatar.scaled(kAvatarSize, kAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  } else {
    ui->avatarTitle->setPixmap(QPixmap(kDefaultAvatar).scaled(kAvatarSize, kAvatarSize));
  }
  ui->nameTitle->setText(name);
}

void MainWindow::setMessageListView(QListView *list_view) {
  DBC_REQUIRE(list_view != nullptr);
  ui->messageListViewLayout->addWidget(list_view);

  message_delegate_ = presenter_->getMessageDelegate();
  list_view->setItemDelegate(message_delegate_);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_upSubmitButton_clicked() {
  SignUpRequest signup_request;
  signup_request.email = ui->upEmail->text().trimmed();
  signup_request.password = ui->upPassword->text().trimmed();
  signup_request.tag = ui->upTag->text().trimmed();
  signup_request.name = ui->upName->text().trimmed();

  auto res = DataInputService::validateRegistrationUserInput(signup_request);
  if (!res.valid)
    showError(res.message);
  else
    presenter_->signUp(signup_request);
}

void MainWindow::on_inSubmitButton_clicked() {
  LogInRequest login_request;
  login_request.email = ui->inEmail->text().trimmed();
  login_request.password = ui->inPassword->text().trimmed();
  auto res = DataInputService::validateLoginUserInput(login_request);
  if (!res.valid) {
    showError(res.message);
  } else {
    presenter_->signIn(login_request);
  }
}

void MainWindow::setMainWindow() const {
  ui->mainStackedWidget->setCurrentIndex(1);
  ui->messageWidget->setVisible(false);
}

void MainWindow::showError(const QString &error) {
  DBC_REQUIRE(!error.isEmpty());
  QMessageBox::warning(this, "ERROR", error);
}

void MainWindow::setupUserListView() {
  if(userListView_) return;

  userListView_ = new ClickOutsideClosableListView(this);
  auto *user_delegate = presenter_->getUserDelegate();
  userListView_->setItemDelegate(user_delegate);

  connect(userListView_, &QListView::clicked, this, [this](const QModelIndex &index) -> void {
    long long user_id = index.data(UserModel::UserIdRole).toLongLong();
    presenter_->onUserClicked(user_id);
  });

  constexpr int maxRows   = 5;
  userListView_->setUpdateCallback([=]() {
    QTimer::singleShot(0, [=]() {
      utils::updateViewVisibility(userListView_,  ui->userTextEdit, utils::Direction::Below, maxRows);
    });
  });
}

void MainWindow::setUserModel(UserModel *user_model) {
  DBC_REQUIRE(user_model != nullptr);
  setupUserListView(); // todo: DBC_REQUIRE(userListView_ != nullptr);

  userListView_->setModel(user_model);
  userListView_->show();
}

void MainWindow::on_userTextEdit_textChanged(const QString &text) {
  if (!text.isEmpty()) presenter_->findUserRequest(text);
}

void MainWindow::on_textEdit_textChanged() {
  constexpr int kMinTextEditHeight = 200;
  constexpr int kAdditionalSpace = 10;
  const int docHeight = static_cast<int>(ui->textEdit->document()->size().height());
  const int new_height = qMin(kMinTextEditHeight, docHeight + kAdditionalSpace);
  ui->textEdit->setFixedHeight(new_height);
}

void MainWindow::on_sendButton_clicked() {
  auto text_to_send = ui->textEdit->toPlainText();

  ui->textEdit->clear();
  presenter_->sendButtonClicked(text_to_send);
}

void MainWindow::clearFindUserEdit() { ui->userTextEdit->clear(); }

void MainWindow::on_logoutButton_clicked() {
  presenter_->onLogOutButtonClicked();
  setSignInPage();
  editable_message_.reset();
}

void MainWindow::setSignInPage() {
  ui->mainStackedWidget->setCurrentIndex(0);
  ui->SignInUpWidget->setCurrentIndex(0);
  ui->SignInButton->setEnabled(false);
  ui->signUpButton->setEnabled(true);
  ui->inEmail->clear();
  ui->inPassword->clear();
  editable_message_.reset();
}

void MainWindow::setSignUpPage() {
  ui->mainStackedWidget->setCurrentIndex(0);
  ui->SignInUpWidget->setCurrentIndex(1);
  ui->SignInButton->setEnabled(true);
  ui->signUpButton->setEnabled(false);
  clearUpInput();
  editable_message_.reset();
}

void MainWindow::clearUpInput() {
  ui->upEmail->clear();
  ui->upName->clear();
  ui->upPassword->clear();
  ui->upTag->clear();
}

void MainWindow::seupConnections() {
  connect(ui->chatListView, &QListView::clicked, this, [this](const QModelIndex &index) -> void {
    long long chat_id = index.data(ChatModel::ChatIdRole).toLongLong();
    presenter_->onChatClicked(chat_id);
  });

  connect(ui->SignInButton, &QPushButton::clicked, this, &MainWindow::setSignInPage);
  connect(ui->signUpButton, &QPushButton::clicked, this, &MainWindow::setSignUpPage);

  DBC_REQUIRE(message_list_view_ != nullptr);
  message_list_view_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(message_list_view_.get(), &MessageListView::clickedWithEvent, this, &MainWindow::onPressEvent);

  connect(presenter_.get(), &Presenter::userSetted, this, &MainWindow::setMainWindow);
  connect(ui->serch_in_chat_button, &QPushButton::clicked, this, &MainWindow::setSearchMessageMode);
  connect(ui->cancel_search_messages_button, &QPushButton::clicked, this, &MainWindow::cancelSearchMessagesMode);
  connect(ui->emojiButton, &QPushButton::toggled, this, &MainWindow::onEmojiButtonStateChanged);
  ui->emojiButton->setCheckable(true);
}

void MainWindow::setupUI() {
  ui->emojiButton->setCheckable(true);
  ui->emojiButton->setChecked(false);
  ui->pushButton->setCheckable(true);
  ui->chatListView->setMouseTracking(true);
  constexpr int kHeightTextEdit = 35;
  setSignInPage();
  ui->textEdit->setFixedHeight(kHeightTextEdit);
  ui->textEdit->setPlaceholderText("Type a message...");

  ui->chatListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->chatListView->setAttribute(Qt::WA_Hover, false);

  ui->chatListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  ui->messageWidget->setVisible(false);
  ui->textEdit->setFrameStyle(QFrame::NoFrame);

  setupUserListView();
  setTheme(std::make_unique<LightTheme>());
}

void MainWindow::setCurrentChatIndex(QModelIndex chat_idx) { ui->chatListView->setCurrentIndex(chat_idx); }

void MainWindow::setTheme(std::unique_ptr<ITheme> theme) {
  DBC_REQUIRE(theme != nullptr);
  current_theme_ = std::move(theme);
  DBC_ENSURE(current_theme_ != nullptr);
  ui->centralwidget->setStyleSheet(current_theme_->getStyleSheet());
}

void MainWindow::onPressEvent(QMouseEvent *event) {
  const QPoint pos = event->pos();

  if (event->button() == Qt::RightButton) {
    onMessageContextMenu(pos);
  } else if (event->button() == Qt::LeftButton) {
    onReactionClicked(pos);
  }

  // MessageListView::mousePressEvent(event);
}

void MainWindow::onReactionClicked(const QPoint &pos) {
  QModelIndex index = message_list_view_->indexAt(pos);
  // todo: message_list_view_->indexAt(pos) 2 times, consider to reallocate it in onPressEvent
  if (!index.isValid()) return;
  Message msg = index.data(MessageModel::Roles::FullMessage).value<Message>();
  if (auto reaction_id = message_delegate_->reactionAt(msg.id, pos); reaction_id.has_value()) {
    presenter_->reactionClicked(msg, reaction_id.value());
  }
}

void MainWindow::on_pushButton_clicked(bool checked) {
  checked ? setTheme(std::make_unique<DarkTheme>()) : setTheme(std::make_unique<LightTheme>());
}

void MainWindow::onMessageContextMenu(const QPoint &pos) {
  QModelIndex index = message_list_view_->indexAt(pos);
  if (!index.isValid()) return;

  Message msg = index.data(MessageModel::Roles::FullMessage).value<Message>();
  auto reactions = presenter_->getDefaultReactionsInChat(msg.chat_id);
  auto *panel = new MessageActionPanel(msg, reactions, this);
  panel->move(message_list_view_->viewport()->mapToGlobal(pos));

  connect(panel, &MessageActionPanel::copyClicked, this, &MainWindow::copyMessage);
  connect(panel, &MessageActionPanel::editClicked, this, &MainWindow::editMessage);
  connect(panel, &MessageActionPanel::deleteClicked, this, &MainWindow::deleteMessage);
  connect(panel, &MessageActionPanel::reactionClicked, [this](const Message &m, long long reaction_id){
    presenter_->reactionClicked(m, reaction_id);
  });

  panel->show();
}

void MainWindow::copyMessage(const Message &message) { qDebug() << "Copy " << message.toString(); }

void MainWindow::editMessage(const Message &message) {
  DBC_REQUIRE(message.isMine() && !message.isOfflineSaved());

  QString text_to_edit = message.text;
  DBC_REQUIRE(!text_to_edit.isEmpty());
  if (text_to_edit.isEmpty()) return;

  ui->inputEditStackedWidget->setCurrentIndex(1);
  ui->editTextEdit->setText(text_to_edit);
  editable_message_ = message;
}

void MainWindow::deleteMessage(const Message &message) {
  DBC_REQUIRE(message.isMine() && message.id > 0);
  presenter_->deleteMessage(message);
}

void MainWindow::on_cancelEditButton_clicked() {
  setWriteMode();
  editable_message_.reset();
}

void MainWindow::on_okEditButton_clicked() {
  QString current_text = ui->editTextEdit->toPlainText();
  DBC_REQUIRE(!current_text.isEmpty());
  DBC_REQUIRE(editable_message_ != std::nullopt);
  setWriteMode();
  ui->editTextEdit->clear();
  Message message_to_update = *editable_message_;
  message_to_update.text = current_text;
  presenter_->updateMessage(message_to_update);
}

void MainWindow::on_editTextEdit_textChanged() {
  QString current_text = ui->editTextEdit->toPlainText();
  ui->okEditButton->setEnabled(!current_text.isEmpty());
}

void MainWindow::setWriteMode() { ui->inputEditStackedWidget->setCurrentIndex(0); }

QModelIndex MainWindow::findIndexByMessageId(QAbstractItemModel *model, long long id) {
  for (int row = 0; row < model->rowCount(); ++row) {
    QModelIndex idx = model->index(row, 0);
    if (idx.data(MessageModel::MessageIdRole).toLongLong() == id) return idx;
  }
  return {};
}

void MainWindow::setupSearchMessageListView() {
  if(searchMessageListView_) return;

  searchMessageListView_ = new ClickOutsideClosableListView(this);
  auto* anchor = ui->search_messages_line_edit;
  constexpr int max_visible_rows = 3;

  searchMessageListView_->setUpdateCallback([=]() {
    utils::updateViewVisibility(searchMessageListView_, anchor, utils::Direction::Below, max_visible_rows);
  });

  searchMessageListView_->setOnCloseCallback([=]() {
    this->cancelSearchMessagesMode();
  });

  searchMessageListView_->addAcceptableClickableWidget(anchor);
}

void MainWindow::setSearchMessageMode() {
  ui->chat_title_stacked_widget->setCurrentIndex(1);
  setupSearchMessageListView();
  searchMessageListView_->setModel(searchResultsModel_.get());
  searchMessageListView_->show();
  ui->search_messages_line_edit->setFocus();
  ui->search_messages_line_edit->selectAll();
}

void MainWindow::setTitleChatMode() { ui->chat_title_stacked_widget->setCurrentIndex(0); }

void MainWindow::cancelSearchMessagesMode() {
  setTitleChatMode();
  ui->search_messages_line_edit->clear();
  searchResultsModel_->clear();
}

void MainWindow::on_search_messages_line_edit_textChanged(const QString &prefix) {
  searchResultsModel_->clear();
  if (prefix.isEmpty()) {
    return;
  }

  auto list_of_message = presenter_->getListOfMessagesBySearch(prefix);  // todo: current_open_id is in presenter;

  if (list_of_message.empty()) {
    QStandardItem *item = new QStandardItem("No results");
    item->setSizeHint(QSize(0, 30));
    searchResultsModel_->appendRow(item);
    return;
  }

  for (const auto &message : list_of_message) {
    QStandardItem *item = new QStandardItem(message.text); // todo: add delegators for this list (icons and some info)

    item->setData(message.id, MessageRoles::MessageIdRole);
    item->setData(message.text, MessageRoles::MessageTextRole);

    searchResultsModel_->appendRow(item);
  }
}

void MainWindow::on_serch_messages_list_view_clicked(const QModelIndex &index) {
  if (!index.isValid()) return;

  long long messageId = index.data(MessageModel::MessageIdRole).toLongLong();
  QModelIndex target = findIndexByMessageId(message_list_view_->model(), messageId);

  if (!target.isValid()) return;

  message_list_view_->scrollTo(target, QAbstractItemView::PositionAtCenter);
  message_list_view_->setCurrentIndex(target);

  // highlightMessage(messageId);
}

QStandardItemModel* MainWindow::getEmojiModel() {
  auto emojiModel = new QStandardItemModel(this);
  auto reactions = presenter_->getReactionsForMenu();
  for (const auto &r : reactions) {
    QStandardItem *item = new QStandardItem;
    QIcon icon(QString::fromStdString(r.image));
    if(icon.isNull()) continue;
    item->setIcon(icon);
    item->setData(r.id, Qt::UserRole + 1);  //todo: already path ReactionInfo
    item->setData(QString::fromStdString(r.image), Qt::UserRole + 2);
    emojiModel->appendRow(item);
  }
  return emojiModel;
}

void MainWindow::openEmojiMenu() { //todo: implement EmojiMenu class as MessageActionPanel
  if(!emoji_menu_) {
    emoji_menu_ = new ClickOutsideClosableListView(this);
    emoji_menu_->setViewMode(QListView::IconMode);
    constexpr int icons_size = 16;
    emoji_menu_->setIconSize(QSize(icons_size,icons_size));

    auto emojiModel = getEmojiModel();
    emoji_menu_->setModel(emojiModel);

    constexpr int max_visible_rows = 6;
    constexpr int items_per_row = 5;

    emoji_menu_->setUpdateCallback([=](){
      utils::updateViewVisibility(emoji_menu_, ui->emojiButton, utils::Direction::Above, max_visible_rows, items_per_row);
    });

    emoji_menu_->setOnCloseCallback([=](){
      this->closeEmojiMenu();
    });

    emoji_menu_->addAcceptableClickableWidget(ui->textEdit);
    emoji_menu_->addAcceptableClickableWidget(ui->emojiButton);

    connect(emoji_menu_, &QListView::clicked, this, [this](const QModelIndex& index) {
      long long emoji_id = index.data(Qt::UserRole + 1).toLongLong();
      QString emoji_path = index.data(Qt::UserRole + 2).toString();
      ReactionInfo emoji(emoji_id, emoji_path.toStdString());
      onEmojiClicked(emoji);
      emoji_menu_->close();
    });
  }

  emoji_menu_->show();
}

void MainWindow::onEmojiClicked(const ReactionInfo& emoji) {
  qDebug() << "Clicked on " << emoji.id << " with path: " << emoji.image;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto* me = static_cast<QMouseEvent*>(event);
    Q_EMIT clickedOnPos(me->globalPos());
  } else if (obj == this && event->type() == QEvent::Resize) {
    Q_EMIT geometryChanged();
  } else if (obj == this && event->type() == QEvent::Move) {
    Q_EMIT geometryChanged();
  }

  return false;
}

void MainWindow::closeEmojiMenu(bool close_manually /* = true */) {
  if (!emoji_menu_) return;

  emoji_menu_->close();

  if(close_manually) {
    ui->emojiButton->blockSignals(true);
    ui->emojiButton->setChecked(false);
    ui->emojiButton->blockSignals(false);
  }
}

void MainWindow::onEmojiButtonStateChanged(bool on) {
  if(on) {
    openEmojiMenu();
  } else {
    closeEmojiMenu(false);
  }
}

