#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>

#include "headers/IMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Presenter;

class MainWindow : public QMainWindow, public IMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setPresenter(Presenter* presenter);
    void setUser(const User& user) override;
    void setChatWindow() override;
    void setChatModel(ChatModel* model) override;
    void setUserModel(UserModel* userModel) override;
    void clearFindUserEdit() override;
    void showError(const QString& error) override;

    void setMessageListView(QListView* listView) override;
    void setCurrentChatIndex(QModelIndex idx) override;

    //int getMaximumMessageScrollBar() override;
    //int getMessageScrollBarValue() override;
    //void setMessageScrollBarValue(int value) override;
    //void setChatInLow() override;

private Q_SLOTS:

    void on_upSubmitButton_clicked();
    void on_inSubmitButton_clicked();
    void on_userTextEdit_textChanged(const QString &text);
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

    Ui::MainWindow *ui;
    Presenter* presenter;
};

#endif // MAINWINDOW_H
