#include "delegators/DelegatorsFactory.h"

#include "delegators/chatitemdelegate.h"
#include "delegators/messagedelegate.h"
#include "delegators/userdelegate.h"
#include "managers/datamanager.h"

DelegatorsFactory::DelegatorsFactory(DataManager *data_manager) : data_manager_(data_manager) {}

MessageDelegate *DelegatorsFactory::getMessageDelegate(QObject *parent) {
  return new MessageDelegate(data_manager_, data_manager_, data_manager_, parent);
}

UserDelegate *DelegatorsFactory::getUserDelegate(QObject *parent) { return new UserDelegate(parent); }

ChatItemDelegate *DelegatorsFactory::getChatDelegate(QObject *parent) {
  return new ChatItemDelegate(data_manager_, parent);
}
