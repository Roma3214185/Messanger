#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include "headers/User.h"
#include "MessageModel/messagemodel.h"
#include "ChatModel/chatmodel.h"
#include "UserModel/UserModel.h"

class IMainWindow
{
public:
    virtual void setUser(const User& user) = 0;
    virtual void setChatWindow(MessageModel* model) = 0;
    virtual void setChatModel(ChatModel* model) = 0;
    virtual void setUserModel(UserModel* userModel) = 0;
    virtual void clearFindUserEdit() = 0;
    virtual void showError(const QString& error) = 0;
};

#endif // IMAINWINDOW_H
