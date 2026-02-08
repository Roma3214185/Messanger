#include <catch2/catch_all.hpp>
#include "handlers/MessageHanldlers.h"
#include "mocks/MockPublisher.h"
#include "mocks/notificationservice/MockUserSocketRepository.h"
#include "mocks/notificationservice/MockSocket.h"

TEST_CASE("Test DeleteMessageReactionHandler") {
    MockPublisher publisher;
    DeleteMessageReactionHandler handler(&publisher);

    SECTION("Handle message with valid saved Reaction expected Publisher call deleteReaction") {
        crow::json::wvalue w;
        int mock_message_id = 12, mock_receiver_id = 1442, reaction_id = 10;
        w["message_id"] = mock_message_id;
        w["receiver_id"] = mock_receiver_id;
        w["reaction_id"] = reaction_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_deleteReaction == 1);
        auto reaction_to_delete = publisher.reactions_to_delete[0];
        reaction_to_delete.message_id = mock_message_id;
        reaction_to_delete.receiver_id = mock_receiver_id;
        reaction_to_delete.reaction_id = reaction_id;
    }

    SECTION("Handle message with invalid saved Reaction expected Publisher no call deleteReaction") {
        crow::json::rvalue msg = crow::json::load(R"({
            "type": "ping"
        })");

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_deleteReaction == 0);
    }
}

TEST_CASE("Test InitMessageHandler") {
    MockUserSocketRepository user_manager;
    InitMessageHandler handler(&user_manager);
    auto socket = std::make_shared<MockSocket>();

    SECTION("Handle message with valid user_id expected expected UserSocketRepositor user_sockets_ not finded socket for  user_id") {
        crow::json::wvalue w;
        int mock_user_id = 12;
        w["user_id"] = mock_user_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, socket);

        REQUIRE(user_manager.user_sockets_.contains(mock_user_id));
    }

    SECTION("Handle message with socket nullptr expected UserSocketRepositor user_sockets_ not finded socket for  user_id") {
        crow::json::wvalue w;
        int mock_user_id = 12;
        w["user_id"] = mock_user_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE_FALSE(user_manager.user_sockets_.contains(mock_user_id));
    }

    SECTION("Handle message with no field user_id expected UserSocketRepositor user_sockets_ not finded socket for  user_id") {
        crow::json::wvalue w;
        int mock_user_id = 12;
        w["user_ids112"] = mock_user_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, socket);

        REQUIRE_FALSE(user_manager.user_sockets_.contains(mock_user_id));
    }

    SECTION("Handle message with invalid user_id expected UserSocketRepositor user_sockets_ not finded socket for  user_id") {
        crow::json::wvalue w;
        int mock_user_id = -1;
        w["user_id"] = mock_user_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, socket);

        REQUIRE_FALSE(user_manager.user_sockets_.contains(mock_user_id));
    }
}

TEST_CASE("Test SaveMessageReactionHandler") {
    MockPublisher publisher;
    SaveMessageReactionHandler handler(&publisher);

    SECTION("Handle message with valid saved Reaction expected Publisher call saveReaction") {
        crow::json::wvalue w;
        int mock_message_id = 12, mock_receiver_id = 1442, reaction_id = 10;
        w["message_id"] = mock_message_id;
        w["receiver_id"] = mock_receiver_id;
        w["reaction_id"] = reaction_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_saveReaction == 1);
        auto reaction_to_save = publisher.reactions_to_save[0];
        reaction_to_save.message_id = mock_message_id;
        reaction_to_save.receiver_id = mock_receiver_id;
        reaction_to_save.reaction_id = reaction_id;
    }
}

TEST_CASE("Test SaveMessageReactionHandler") {
    MockPublisher publisher;
    SaveMessageReactionHandler handler(&publisher);
    SECTION("Handle message with invalid saved Reaction expected Publisher no call deleteReaction") {
        crow::json::rvalue msg = crow::json::load(R"({
            "type": "ping"
        })");

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_saveReaction == 0);
    }
}

TEST_CASE("Test MarkReadMessageHandler") {
    MockPublisher publisher;
    MarkReadMessageHandler handler(&publisher);

    SECTION("Handle message with valid saved MessageStatus expected Publisher call saveMessageStatus") {
        crow::json::wvalue w;
        int mock_message_id = 12, mock_receiver_id = 1442;
        w["message_id"] = mock_message_id;
        w["readed_by"] = mock_receiver_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_saveMessageStatus == 1);
        auto status_to_save = publisher.messages_status_to_save[0];
        status_to_save.message_id = mock_message_id;
        status_to_save.receiver_id = mock_receiver_id;
    }

    SECTION("Handle message without message_id field expected Publisher no call saveMessageStatus") {
        crow::json::wvalue w;
        int mock_message_id = 12, mock_receiver_id = 1442;
        w["readed_by"] = mock_receiver_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_saveMessageStatus == 0);
    }

    SECTION("Handle message without readed_by field expected Publisher no call saveMessageStatus") {
        crow::json::wvalue w;
        int mock_message_id = 12, mock_receiver_id = 1442;
        w["message_id"] = mock_message_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_saveMessageStatus == 0);
    }
}

TEST_CASE("Test SendMessageHandler") {
    MockPublisher publisher;
    SendMessageHandler handler(&publisher);
    Message expected;
    expected.chat_id = 12;
    expected.local_id = "1101";
    expected.text = "13131";
    expected.sender_id = 1213;


    SECTION("Handle message expected Publisher call saveMessage") {
        crow::json::wvalue w;
        w["chat_id"] = expected.chat_id;
        w["text"] = expected.text;
        w["sender_id"] = expected.sender_id;
        w["timestamp"] = expected.timestamp;
        w["local_id"] = expected.local_id;
        crow::json::rvalue msg = crow::json::load(w.dump());

        handler.handle(msg, nullptr);

        REQUIRE(publisher.calls_saveMessage == 1);
        REQUIRE(publisher.messages_to_save.size() == 1);
        auto message_to_save = publisher.messages_to_save[0];
        message_to_save.chat_id = message_to_save.chat_id;
        message_to_save.local_id = expected.local_id;
        message_to_save.sender_id = expected.sender_id;
        message_to_save.text = expected.text;
        message_to_save.timestamp = expected.timestamp;
    }
}

