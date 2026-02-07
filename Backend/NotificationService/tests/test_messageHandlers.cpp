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

/*

inline std::optional<Reaction> parseReaction(
    const crow::json::rvalue &json) {  // todo: make just common crow::json::rwalue -> nlohmann::json
  Reaction reaction;
  reaction.message_id = json["message_id"].i();
  reaction.receiver_id = json["receiver_id"].i();
  reaction.reaction_id = json["reaction_id"].i();
  return reaction.checkInvariants() ? std::make_optional(reaction) : std::nullopt;
}

*/
