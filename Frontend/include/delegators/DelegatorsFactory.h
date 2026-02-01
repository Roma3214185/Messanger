#ifndef DELEGATORSFACTORY_H
#define DELEGATORSFACTORY_H

class MessageDelegate;
class UserDelegate;
class ChatItemDelegate;
class TokenManager;
class DataManager;
class QObject;

class DelegatorsFactory {
  DataManager *data_manager_;

 public:
  DelegatorsFactory(DataManager *data_manager);
  MessageDelegate *getMessageDelegate(QObject *parent);
  UserDelegate *getUserDelegate(QObject *parent);
  ChatItemDelegate *getChatDelegate(QObject *parent);
};

#endif  // DELEGATORSFACTORY_H
