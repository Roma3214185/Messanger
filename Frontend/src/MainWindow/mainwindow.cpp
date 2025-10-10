#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/SignUpRequest.h"
#include "MessageDelegate/messagedelegate.h"
#include "MessageModel/messagemodel.h"
#include "ChatItemDelegate/chatitemdelegate.h"
#include "ChatModel/chatmodel.h"
#include "UserDelegate/userdelegate.h"
#include "DataInputService/datainputservice.h"
#include <QMessageBox>
#include "Presenter/presenter.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setDelegators();
    ui->chatListView->setMouseTracking(true);

    connect(ui->chatListView, &QListView::clicked, this, [=](const QModelIndex &index){
        auto chatId = index.data(ChatModel::ChatIdRole).toInt();
        qDebug() << "Clicked chat:" << chatId;
        presenter->on_chat_clicked(chatId);
    });

    connect(ui->userListView, &QListView::clicked, this, [=](const QModelIndex &index){
        auto userId = index.data(UserModel::UserIdRole).toInt();
        qDebug() << "Clicked on user:" << userId;
        presenter->on_user_clicked(userId);
    });

    connect(ui->SignInButton, &QPushButton::clicked, this, &MainWindow::setSignInPage);
    connect(ui->signUpButton, &QPushButton::clicked, this, &MainWindow::setSignUpPage);

    setSignInPage();
    ui->textEdit->setFixedHeight(35);
    ui->textEdit->setPlaceholderText("Type a message...");

    ui->chatListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->userListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->messageWidget->setVisible(false);
    ui->textEdit->setFrameStyle(QFrame::NoFrame);

    setSignInPage();
    ui->messageListView->setFocusPolicy(Qt::NoFocus);
    ui->messageListView->setSelectionMode(QAbstractItemView::NoSelection);
}

void MainWindow::setDelegators(){
    auto* chatDelegate = new ChatItemDelegate(this);
    auto* messageDelegate = new MessageDelegate(this);
    auto* userDelegate = new UserDelegate(this);

    ui->chatListView->setItemDelegate(chatDelegate);
    ui->messageListView->setItemDelegate(messageDelegate);
    ui->userListView->setItemDelegate(userDelegate);
}

void MainWindow::setChatModel(ChatModel* model) {
    ui->chatListView->setModel(model);
}

void MainWindow::setChatWindow(MessageModel* model){
    ui->messageWidget->setVisible(true);
    ui->messageListView->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPresenter(Presenter* presenter)
{
    this->presenter = presenter;
}

void MainWindow::on_upSubmitButton_clicked()
{
    auto email = ui->upEmail->text();
    auto password = ui->upPassword->text();
    auto tag = ui->upTag->text();
    auto name = ui->upName->text();

    if(!DataInputService::emailValid(email)){
        QMessageBox::warning(this, "Invalid email", "Email is invalid");
        return;
    }

    if(!DataInputService::passwordValid(password)){
        QMessageBox::warning(this, "Invalid password", "Password is invalid");
        return;
    }

    if(!DataInputService::tagValid(tag)){
        QMessageBox::warning(this, "Invalid tag", "Tag is invalid");
        return;
    }

    if(!DataInputService::nameValid(name)){
        QMessageBox::warning(this, "Invalid name", "Name is invalid");
        return;
    }

    SignUpRequest req{
        .email = email,
        .password = password,
        .tag = tag,
        .name = name
    };
    presenter->signUp(req);
}



void MainWindow::on_inSubmitButton_clicked()
{
    auto email = ui->inEmail->text();
    auto password = ui->inPassword->text();

    if(!DataInputService::emailValid(email)){
        QMessageBox::warning(this, "Invalid email", "Email is invalid");
        return;
    }

    if(!DataInputService::passwordValid(password)){
        QMessageBox::warning(this, "Invalid password", "Password is invalid");
        return;
    }

    presenter->signIn(email, password);
}

void MainWindow::setMainWindow(){
    ui->mainStackedWidget->setCurrentIndex(1);
    ui->messageWidget->setVisible(false);
}

void MainWindow::setUser(User user){
    setMainWindow();
    qDebug() << "[INFO] Hi user " << user.email << " " << user.name << " " << user.id << " " << user.tag;
}

void MainWindow::setUserModel(UserModel* userModel){
    ui->userListView->setModel(userModel);
}

void MainWindow::on_userTextEdit_textChanged(const QString &text)
{
    presenter->findUserRequest(text);
}


void MainWindow::on_textEdit_textChanged()
{
    auto docHeight = ui->textEdit->document()->size().height();
    int newHeight = qMin(200, static_cast<int>(docHeight + 10));
    ui->textEdit->setFixedHeight(newHeight);
}

void MainWindow::on_sendButton_clicked()
{
    auto textToSend = ui->textEdit->toPlainText();

    ui->textEdit->clear();
    presenter->sendButtonClicked(textToSend);
}

void MainWindow::clearFindUserEdit(){
    ui->userTextEdit->clear();
}

void MainWindow::on_logoutButton_clicked()
{
    presenter->on_logOutButtonClicked();
    setSignInPage();
}

void MainWindow::setSignInPage(){
    ui->mainStackedWidget->setCurrentIndex(0);
    ui->SignInUpWidget->setCurrentIndex(0);
    ui->SignInButton->setEnabled(false);
    ui->signUpButton->setEnabled(true);
    ui->inEmail->clear();
    ui->inPassword->clear();
}

void MainWindow::setSignUpPage(){
    ui->mainStackedWidget->setCurrentIndex(0);
    ui->SignInUpWidget->setCurrentIndex(1);
    ui->SignInButton->setEnabled(true);
    ui->signUpButton->setEnabled(false);
    clearUpInput();
}

void MainWindow::clearUpInput(){
    ui->upEmail->clear();
    ui->upName->clear();
    ui->upPassword->clear();
    ui->upTag->clear();
}

