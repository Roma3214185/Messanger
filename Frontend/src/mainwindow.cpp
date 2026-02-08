#include "ui/mainwindow.h"

#include <QFrame>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>
#include <cstdint>

#include "../forms/ui_mainwindow.h"
#include "Debug_profiling.h"
#include "ui/MessageActionPanel.h"
#include "ui/MessageListView.h"
#include "ui/utilsui.h"
#include "ui/clickoutsideclosablelistview.h"
#include "delegators/DelegatorsFactory.h"
#include "delegators/chatitemdelegate.h"
#include "delegators/messagedelegate.h"
#include "delegators/userdelegate.h"
#include "dto/SignUpRequest.h"
#include "interfaces/ITheme.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"
#include "presenter.h"
#include "utils.h"

namespace MessageRoles {
enum { message_idRole = Qt::UserRole + 1, MessageTextRole };
}

MainWindow::MainWindow(Model *model, DelegatorsFactory *delegators_factory, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      delegators_factory_(delegators_factory),
      message_list_view_(std::make_unique<MessageListView>()),
      search_results_model_(std::make_unique<MessageModel>(this)) {
  ui->setupUi(this);
  qApp->installEventFilter(this);
}

void MainWindow::setPresenter(Presenter *presenter) { presenter_ = presenter; }

void MainWindow::initialise() {
  if(presenter_) {
    throw std::runtime_error("Presenter not initialised");
  }
  presenter_->initialise();

  setDelegators();
  setupConnections();
  setupUI();
  setSignInPage();
  setWriteMode();
}

void MainWindow::setDelegators() {
  auto *chat_delegate = delegators_factory_->getChatDelegate(ui->chatListView);
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

  auto avatar = utils::ui::getPixmapFromPath(chat->avatar_path);
  constexpr int kAvatarSize = 40;
  ui->avatarTitle->setPixmap(avatar.scaled(kAvatarSize, kAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  ui->nameTitle->setText(chat->title);
}

void MainWindow::setMessageListView() {
  presenter_->setMessageListView(message_list_view_.get());
  ui->messageListViewLayout->addWidget(message_list_view_.get());
  message_delegate_ = delegators_factory_->getMessageDelegate(message_list_view_.get());
  message_list_view_->setItemDelegate(message_delegate_);
  connect(message_delegate_, &MessageDelegate::unreadMessage, this,
          [this](Message &message) { presenter_->onUnreadMessage(message); });
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_upSubmitButton_clicked() {
  SignUpRequest signup_request;
  signup_request.email = ui->upEmail->text().trimmed();
  signup_request.password = ui->upPassword->text().trimmed();
  signup_request.tag = ui->upTag->text().trimmed();
  signup_request.name = ui->upName->text().trimmed();
  presenter_->signUp(signup_request);
}

void MainWindow::on_inSubmitButton_clicked() {
  LogInRequest login_request;
  login_request.email = ui->inEmail->text().trimmed();
  login_request.password = ui->inPassword->text().trimmed();
  presenter_->signIn(login_request);
}

void MainWindow::setMainWindow() const {
  ui->mainStackedWidget->setCurrentIndex(1);
}

void MainWindow::closeChatWidget() {
  ui->messageWidget->setVisible(false);
}

void MainWindow::showError(const QString &error) {
  DBC_REQUIRE(!error.isEmpty());
  QMessageBox::warning(this, "ERROR", error);
}

void MainWindow::setupUserListView() {
  if (user_list_view_) return;

  user_list_view_ = new ClickOutsideClosableListView(this);
  user_list_view_->setItemDelegate(delegators_factory_->getUserDelegate(user_list_view_));

  connect(user_list_view_, &QListView::clicked, this, [this](const QModelIndex &index) -> void {
    clearFindUserEdit();
    long long user_id = index.data(UserModel::UserIdRole).toLongLong();
    presenter_->onUserClicked(user_id);
  });

  constexpr int kMaxVisibleRows = 5;
  user_list_view_->setUpdateCallback([=]() {
    QTimer::singleShot(0, [=]() {
      utils::ui::updateViewVisibility(user_list_view_, ui->userTextEdit, utils::ui::Direction::Below, kMaxVisibleRows);
    });
  });
}

void MainWindow::setUserModel(UserModel *user_model) {
  DBC_REQUIRE(user_model != nullptr);
  setupUserListView();
  user_list_view_->setModel(user_model);
  user_list_view_->show();
}

void MainWindow::on_userTextEdit_textChanged(const QString &text) {
  if (!text.isEmpty()) presenter_->findUserRequest(text);
}

void MainWindow::on_textEdit_textChanged() {
  constexpr int kMinTextEditHeight = 200;
  constexpr int kAdditionalSpace = 10;
  const int doc_height = static_cast<int>(ui->textEdit->document()->size().height());
  const int new_height = qMin(kMinTextEditHeight, doc_height + kAdditionalSpace);
  ui->textEdit->setFixedHeight(new_height);
}

void MainWindow::on_sendButton_clicked() {
  presenter_->sendButtonClicked(ui->textEdit->document(), answer_on_message_);
  ui->textEdit->clear();
  resetMessageAnswerOnMode();
}

void MainWindow::clearFindUserEdit() { ui->userTextEdit->clear(); }

void MainWindow::on_logoutButton_clicked() {
  presenter_->onLogOutButtonClicked();
  editable_message_.reset();
  answer_on_message_.reset();
  current_theme_.reset();
  message_list_view_.reset();
  search_results_model_->clear();
  setSignInPage();
}

void MainWindow::setSignInPage() {
  ui->mainStackedWidget->setCurrentIndex(0);
  ui->SignInUpWidget->setCurrentIndex(0);
  ui->SignInButton->setEnabled(false);
  ui->signUpButton->setEnabled(true);
  clearSignInPage();
}

void MainWindow::clearSignInPage() {
    ui->inEmail->clear();
    ui->inPassword->clear();
}

void MainWindow::setSignUpPage() {
  ui->mainStackedWidget->setCurrentIndex(0);
  ui->SignInUpWidget->setCurrentIndex(1);
  ui->SignInButton->setEnabled(true);
  ui->signUpButton->setEnabled(false);
  clearSignUpPage();
}

void MainWindow::clearSignUpPage() {
  ui->upEmail->clear();
  ui->upName->clear();
  ui->upPassword->clear();
  ui->upTag->clear();
}

void MainWindow::setupConnections() {
  DBC_REQUIRE(message_list_view_ != nullptr);

  connect(ui->chatListView, &QListView::clicked, this, [this](const QModelIndex &index) -> void {
    long long chat_id = index.data(ChatModel::ChatIdRole).toLongLong();
    presenter_->onChatClicked(chat_id);
  });
  connect(ui->SignInButton, &QPushButton::clicked, this, &MainWindow::setSignInPage);
  connect(ui->signUpButton, &QPushButton::clicked, this, &MainWindow::setSignUpPage);
  connect(message_list_view_.get(), &MessageListView::clickedWithEvent, this, &MainWindow::on_PressEvent);
  connect(presenter_, &Presenter::userSetted, this, &MainWindow::setMainWindow);
  connect(presenter_, &Presenter::userSetted, this, &MainWindow::closeChatWidget);
  connect(ui->serch_in_chat_button, &QPushButton::clicked, this, &MainWindow::setSearchMessageMode);
  connect(ui->cancel_search_messages_button, &QPushButton::clicked, this, &MainWindow::cancelSearchMessagesMode);
  connect(ui->emojiButton, &QPushButton::toggled, this, &MainWindow::on_EmojiButtonStateChanged);
}

void MainWindow::setupUI() {
  ui->emojiButton->setCheckable(true);
  ui->emojiButton->setChecked(false);
  ui->themeButton->setCheckable(true);
  ui->chatListView->setMouseTracking(true);

  constexpr int kHeightTextEdit = 35;
  ui->textEdit->setFixedHeight(kHeightTextEdit);
  ui->textEdit->setPlaceholderText("Type a message...");

  ui->chatListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->chatListView->setAttribute(Qt::WA_Hover, false);

  ui->chatListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  ui->messageWidget->setVisible(false);
  ui->textEdit->setFrameStyle(QFrame::NoFrame);

  setupUserListView();
  setMessageListView();
  setTheme(std::make_unique<LightTheme>());
}

void MainWindow::setCurrentChatIndex(QModelIndex chat_idx) { ui->chatListView->setCurrentIndex(chat_idx); }

void MainWindow::setTheme(std::unique_ptr<ITheme> theme) {
  DBC_REQUIRE(theme != nullptr);
  current_theme_ = std::move(theme);
  ui->centralwidget->setStyleSheet(current_theme_->getStyleSheet());
}

void MainWindow::on_PressEvent(QMouseEvent *event) {
  const QPoint pos = event->pos();

  if (event->button() == Qt::RightButton) {
    on_MessageContextMenu(pos);
  } else if (event->button() == Qt::LeftButton) {
    on_ReactionClicked(pos);
  }
}

void MainWindow::on_ReactionClicked(const QPoint &pos) {
  QModelIndex index = message_list_view_->indexAt(pos);
  if (!index.isValid()) return;

  const auto message_to_set_reaction = index.data(MessageModel::Roles::FullMessage).value<Message>();
  long long message_id = message_to_set_reaction.id;

  if (auto reaction_id = message_delegate_->reactionAt(message_id, pos); reaction_id.has_value()) {
    presenter_->reactionClicked(message_to_set_reaction, reaction_id.value());
  }
}

void MainWindow::on_themeButton_clicked(bool checked) {
  checked ? setTheme(std::make_unique<DarkTheme>()) : setTheme(std::make_unique<LightTheme>());
}

void MainWindow::on_MessageContextMenu(const QPoint &pos) {
  QModelIndex index = message_list_view_->indexAt(pos);
  if(!index.isValid()) return;

  const auto message_clicked = index.data(MessageModel::Roles::FullMessage).value<Message>();
  auto reactions = presenter_->getDefaultReactionsInChat(message_clicked.chat_id);
  auto *panel = new MessageActionPanel(message_clicked, reactions, this);
  panel->move(message_list_view_->viewport()->mapToGlobal(pos));

  connect(panel, &MessageActionPanel::onAnswerClicked, this, &MainWindow::setMessageAnswerOnMode);
  connect(panel, &MessageActionPanel::copyClicked, this, &MainWindow::copyMessage);
  connect(panel, &MessageActionPanel::editClicked, this, &MainWindow::editMessage);
  connect(panel, &MessageActionPanel::deleteClicked, this, &MainWindow::deleteMessage);
  connect(panel, &MessageActionPanel::reactionClicked,
          [this](const Message &m, long long reaction_id) { presenter_->reactionClicked(m, reaction_id); });

  panel->show();
}

void MainWindow::setMessageAnswerOnMode(const Message &message_to_answer) {
  if (message_to_answer.isOfflineSaved()) return;
  utils::ui::clearLayout(ui->answer_on_layout);
  answer_on_message_ = message_to_answer.id;

  auto *model = new MessageModel(this);
  model->saveMessage(message_to_answer);

  auto *list_view = new QListView(this);
  list_view->setModel(model);
  list_view->setFixedHeight(70);
  list_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  list_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  connect(list_view, &QListView::clicked, this, &MainWindow::scrollСhatToMessage);

  auto *delegate = delegators_factory_->getMessageDelegate(list_view);
  delegate->setDrawAnswerOn(false);
  delegate->setSaveHitboxes(false);
  delegate->setDrawReactions(false);
  list_view->setItemDelegate(delegate);

  auto *cancel_button = new QPushButton(this);
  cancel_button->setText("X");
  connect(cancel_button, &QPushButton::clicked, this, &MainWindow::resetMessageAnswerOnMode);

  ui->answer_on_layout->addWidget(list_view);
  ui->answer_on_layout->addWidget(cancel_button);
}

void MainWindow::resetMessageAnswerOnMode() {
  utils::ui::clearLayout(ui->answer_on_layout);
  answer_on_message_.reset();
}

void MainWindow::copyMessage(const Message &message) { qDebug() << "Copy " << message.toString(); }

void MainWindow::editMessage(const Message &message) {
  if(!message.isMine() || message.isOfflineSaved()) {
    DBC_UNREACHABLE();
    return;
  }

  if (message.tokens.empty()) return;
  ui->inputEditStackedWidget->setCurrentIndex(1);
  ui->editTextEdit->clear();
  // todo: always save in chat entity user currently input as tokens

  auto cursor = ui->editTextEdit->textCursor();
  insertTokens(cursor, message.tokens);
  editable_message_ = message;
}

void MainWindow::insertTokens(QTextCursor& cursor, const std::vector<MessageToken>& message_tokens) {
    if (message_tokens.empty()) return;

    for (const auto &token : message_tokens) {
        if (token.type == MessageTokenType::Text) {
            cursor.insertText(token.value);
        } else if (token.type == MessageTokenType::Emoji) {
            DBC_REQUIRE(token.emoji_id.has_value());
            long long emoji_id = token.emoji_id.value();
            auto img_info_opt = presenter_->getReactionInfo(emoji_id);
            utils::ui::insert_emoji(cursor, img_info_opt);
        } else {
            DBC_UNREACHABLE();
            return;
        }
    }
}

void MainWindow::deleteMessage(const Message &message) {
  DBC_REQUIRE(message.isMine() && !message.isOfflineSaved());
  //todo: if !message.isMine() check permission to delete,
  //todo: if message.isOfflineSaved() add cancel sending
  presenter_->deleteMessage(message);
}

void MainWindow::on_cancelEditMessageButton_clicked() {
  setWriteMode();
  editable_message_.reset();
}

void MainWindow::on_okEditButton_clicked() {
  DBC_REQUIRE(editable_message_ != std::nullopt);
  QString current_text = ui->editTextEdit->toPlainText();
  DBC_REQUIRE(!current_text.isEmpty());

  presenter_->editMessage(*editable_message_, ui->editTextEdit->document());
  ui->editTextEdit->clear();
  setWriteMode();
}

void MainWindow::on_editTextEdit_textChanged() {
  QString current_text = ui->editTextEdit->toPlainText();
  ui->okEditButton->setEnabled(!current_text.isEmpty());
}

void MainWindow::setWriteMode() { ui->inputEditStackedWidget->setCurrentIndex(0); }

void MainWindow::setupSearchMessageListView() {
  if (search_message_list_view_) {
    if(search_results_model_) search_message_list_view_->setModel(search_results_model_.get());
    return;
  }

  auto *anchor = ui->search_messages_line_edit;
  constexpr int max_visible_rows = 3;

  search_message_list_view_ = new ClickOutsideClosableListView(this);
  auto *message_delegate = delegators_factory_->getMessageDelegate(search_message_list_view_);
  search_message_list_view_->setItemDelegate(message_delegate);

  search_message_list_view_->setUpdateCallback([=]() {
    utils::ui::updateViewVisibility(search_message_list_view_, anchor, utils::ui::Direction::Below, max_visible_rows);
  });

  search_message_list_view_->setOnCloseCallback([=]() { this->cancelSearchMessagesMode(); });
  search_message_list_view_->addAcceptableClickableWidget(anchor);
  connect(search_message_list_view_, &QListView::clicked, this, &MainWindow::on_serch_messages_list_view_clicked);
  if(search_results_model_) search_message_list_view_->setModel(search_results_model_.get());
}

void MainWindow::setSearchMessageMode() {
  ui->chat_title_stacked_widget->setCurrentIndex(1);
  setupSearchMessageListView();
  ui->search_messages_line_edit->setFocus();
  ui->search_messages_line_edit->selectAll();
  search_message_list_view_->show();
}

void MainWindow::setTitleChatMode() { ui->chat_title_stacked_widget->setCurrentIndex(0); }

void MainWindow::cancelSearchMessagesMode() {
  ui->search_messages_line_edit->clear();
  search_results_model_->clear();
  setTitleChatMode();
}

void MainWindow::on_search_messages_line_edit_textChanged(const QString &message_to_search_users) {
  search_results_model_->clear();
  auto list_of_message = presenter_->getListOfMessagesBySearch(message_to_search_users);
  for (const auto &message : list_of_message) {
    search_results_model_->saveMessage(message);
  }
}

void MainWindow::scrollСhatToMessage(const QModelIndex &index_to_scroll) {
  message_list_view_->scrollToMessage(index_to_scroll);
  // todo: highlightMessage(messageId) using delegate
}

void MainWindow::on_serch_messages_list_view_clicked(const QModelIndex &index_clicked) {
  if (!index_clicked.isValid()) return;
  scrollСhatToMessage(index_clicked);
  search_results_model_->clear();
  search_message_list_view_->close();
}

QStandardItemModel *MainWindow::getEmojiModel() {
  auto *emojiModel = new QStandardItemModel(this);
  auto reactions = presenter_->getReactionsForMenu();
  for (const auto &r : reactions) {
    auto *item = new QStandardItem();
    QIcon icon(QString::fromStdString(r.image));
    if (icon.isNull()) continue;
    item->setIcon(icon);
    item->setData(r.id, Qt::UserRole + 1);
    item->setData(QString::fromStdString(r.image), Qt::UserRole + 2);
    emojiModel->appendRow(item);
  }
  return emojiModel;
}

void MainWindow::setupEmojiMenu() {
  if (emoji_menu_) return;

  emoji_menu_ = new ClickOutsideClosableListView(this);
  emoji_menu_->setViewMode(QListView::IconMode);
  constexpr int icons_size = 16;
  emoji_menu_->setIconSize(QSize(icons_size, icons_size));
  emoji_menu_->setModel(getEmojiModel());

  emoji_menu_->setUpdateCallback([=]() {
    constexpr int max_visible_rows = 6;
    constexpr int items_per_row = 5;
    utils::ui::updateViewVisibility(emoji_menu_, ui->emojiButton, utils::ui::Direction::Above, max_visible_rows,
                                    items_per_row);
  });

  emoji_menu_->setOnCloseCallback([=]() { this->closeEmojiMenu(); });
  emoji_menu_->addAcceptableClickableWidget(ui->textEdit);
  emoji_menu_->addAcceptableClickableWidget(ui->emojiButton);

  connect(emoji_menu_, &QListView::clicked, this, [this](const QModelIndex &index) {
    const auto emoji_id = index.data(Qt::UserRole + 1).toLongLong();
    const auto emoji_path = index.data(Qt::UserRole + 2).toString();
    ReactionInfo emoji{emoji_id, emoji_path.toStdString()};
    on_EmojiClicked(emoji);
    emoji_menu_->close();
  });
}

void MainWindow::openEmojiMenu() {
  setupEmojiMenu();
  emoji_menu_->show();
}

void MainWindow::on_EmojiClicked(const ReactionInfo &emoji) {
  QTextCursor cursor = ui->textEdit->textCursor();
  utils::ui::insert_emoji(cursor, emoji);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto *me = static_cast<QMouseEvent *>(event);
    Q_EMIT clickedOnScreenPosition(me->globalPos());
  } else if (obj == this && event->type() == QEvent::Resize) {
    Q_EMIT screenGeometryChanged();
  } else if (obj == this && event->type() == QEvent::Move) {
    Q_EMIT screenGeometryChanged();
  }

  return false;
}

void MainWindow::closeEmojiMenu(bool close_manually) {
  if (!emoji_menu_) return;
  emoji_menu_->close();

  if (close_manually) {
    ui->emojiButton->blockSignals(true);
    ui->emojiButton->setChecked(false);
    ui->emojiButton->blockSignals(false);
  }
}

void MainWindow::on_EmojiButtonStateChanged(bool open_emoji_meu) {
  if (open_emoji_meu) {
    openEmojiMenu();
  } else {
    closeEmojiMenu(false);
  }
}
