#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Presenter/presenter.h>
#include <QListView>
#include "headers/IMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public IMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setPresenter(Presenter* presenter);
    void setUser(User user) override;
    void setChatWindow(MessageModel* model) override;
    void setChatModel(ChatModel* model) override;
    void setUserModel(UserModel* userModel) override;
    void clearFindUserEdit() override;
private:
    void setMainWindow();
    void setSignInPage();
    void setSignUpPage();
    void clearUpInput();

private Q_SLOTS:
    void on_upSubmitButton_clicked();
    void on_inSubmitButton_clicked();
    void on_userTextEdit_textChanged(const QString &text);
    void on_textEdit_textChanged();
    void on_sendButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::MainWindow *ui;
    Presenter* presenter;
};


#endif // MAINWINDOW_H
