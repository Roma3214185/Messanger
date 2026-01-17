#include <QStandardItem>
#include <catch2/catch_all.hpp>

#include "delegators/chatitemdelegate.h"

struct TestChatItemDelegate : public ChatItemDelegate {
  using ChatItemDelegate::ChatItemDelegate;
  using ChatItemDelegate::extractChatData;

};

TEST_CASE("ChatItemDelegate::extractChatData") {
  DataManager data_manager;
  TestChatItemDelegate delegate(&data_manager);
  Message message;
  std::string last_message = "Last_message";
  message.id = 12;
  message.chat_id = 11;
  message.local_id = "123";
  message.receiver_id = 121;
  message.sender_id = 1213;
  auto token = TokenFactory::createTextToken(QString::fromStdString(last_message));
  message.tokens.push_back(token);

  QStandardItemModel model;
  QStandardItem *item = new QStandardItem();
  item->setData("Chat Title", ChatModel::TitleRole);
  item->setData(QVariant::fromValue(message), ChatModel::LastMessageRole);
  item->setData("avatar.png", ChatModel::AvatarRole);
  item->setData(3, ChatModel::UnreadRole);
  model.appendRow(item);

  QModelIndex index = model.index(0, 0);
  ChatDrawData data = delegate.extractChatData(index);

  REQUIRE(data.title == "Chat Title");
  REQUIRE(data.last_message.has_value());
  REQUIRE(data.last_message.value().getFullText().toStdString() == last_message);
  REQUIRE(data.avatar_path == "avatar.png");
  REQUIRE(data.unread == 3);
}

TEST_CASE("ChatItemDelegate sizeHint returns configured size") {
  ChatItemStyle style;
  style.chat_item_width = 300;
  style.chat_item_height = 80;
  ChatItemDelegate delegate(nullptr, nullptr, style);

  QSize size = delegate.sizeHint(QStyleOptionViewItem{}, QModelIndex{});
  REQUIRE(size.width() == 300);
  REQUIRE(size.height() == 300);
}
