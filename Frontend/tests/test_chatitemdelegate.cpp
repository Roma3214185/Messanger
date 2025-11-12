#include <catch2/catch_all.hpp>
#include <QStandardItem>

#include "delegators/chatitemdelegate.h"

struct TestChatItemDelegate : public ChatItemDelegate {
    using ChatItemDelegate::ChatItemDelegate;
    using ChatItemDelegate::extractChatData;
    using ChatItemDelegate::refactorLastMessage;
};


TEST_CASE("ChatItemDelegate::refactorLastMessage") {
  ChatItemStyle style;
  TestChatItemDelegate delegate(nullptr, style);

  SECTION("empty message returns default text") {
    REQUIRE(delegate.refactorLastMessage("") == style.no_message_status);
  }

  SECTION("short message remains unchanged") {
    REQUIRE(delegate.refactorLastMessage("Hello") == "Hello");
  }

  SECTION("long message is truncated") {
    QString long_msg = "This is a very long message that should be truncated";
    QString result   = delegate.refactorLastMessage(long_msg);
    REQUIRE(result.length() <= 25);
    REQUIRE(result.endsWith("..."));
  }
}

TEST_CASE("ChatItemDelegate::extractChatData") {
  TestChatItemDelegate delegate;

  QStandardItemModel model;
  QStandardItem* item = new QStandardItem();
  item->setData("Chat Title", ChatModel::TitleRole);
  item->setData("Last message", ChatModel::LastMessageRole);
  item->setData("avatar.png", ChatModel::AvatarRole);
  item->setData(QDateTime(QDate(2025, 11, 12), QTime(12, 0)), ChatModel::LastMessageTimeRole);
  item->setData(3, ChatModel::UnreadRole);
  model.appendRow(item);

  QModelIndex index = model.index(0, 0);
  ChatDrawData data = delegate.extractChatData(index);

  REQUIRE(data.title == "Chat Title");
  REQUIRE(data.last_message == "Last message");
  REQUIRE(data.avatar_path == "avatar.png");
  REQUIRE(data.unread == 3);
  REQUIRE(data.time == QDateTime(QDate(2025, 11, 12), QTime(12, 0)));
}

TEST_CASE("ChatItemDelegate sizeHint returns configured size") {
  ChatItemStyle style;
  style.chat_item_width = 300;
  style.chat_item_height = 80;
  ChatItemDelegate delegate(nullptr, style);

  QSize size = delegate.sizeHint(QStyleOptionViewItem{}, QModelIndex{});
  REQUIRE(size.width() == 300);
  REQUIRE(size.height() == 300);
}
