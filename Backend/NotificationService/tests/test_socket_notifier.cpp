#include <catch2/catch_all.hpp>
#include "notificationservice/SocketNotifier.h"
#include "notificationservice/SocketRepository.h"
#include "mocks/notificationservice/MockSocket.h"

class MockUserSocketRepository : public IUserSocketRepository {
public:
    UserSocketsMap user_sockets_;

    void saveConnections(UserId user_id, SocketPtr socket) override {
        user_sockets_[user_id] = socket;
    }

    SocketPtr getUserSocket(UserId user_id) override {
        return user_sockets_.contains(user_id) ? user_sockets_[user_id] : nullptr;
    }

    bool userOnline(UserId user_id) override {
        return true;
    }
};

struct SocketNotifierTestFixture {
    MockUserSocketRepository socket_repository;
    SocketNotifier socket_notifier;
    int user_id{1213};
    nlohmann::json json_to_send;
    std::string type{"12131_type"};

    SocketNotifierTestFixture() : socket_notifier(&socket_repository) {
        json_to_send["message"] = "Text";
    }
};

TEST_CASE("Test socket notifier with empty socket repository") {
    SocketNotifierTestFixture fix;

    SECTION("Socket not found expected return false") {
        REQUIRE_FALSE(fix.socket_notifier.notifyMember(fix.user_id, fix.json_to_send, ""));
    }
}

TEST_CASE("Test socket notifier with saved socket repository") {
    SocketNotifierTestFixture fix;
    auto socket = std::make_shared<MockSocket>();
    fix.socket_repository.saveConnections(fix.user_id, socket);

    SECTION("Socket with found expected return true") {
        REQUIRE(fix.socket_notifier.notifyMember(fix.user_id, fix.json_to_send, fix.type));
    }

    SECTION("Socket with sending null json message expected return false") {
        nlohmann::json empty_json;
        REQUIRE_FALSE(fix.socket_notifier.notifyMember(fix.user_id, empty_json, fix.type));
    }

    SECTION("Socket with found expected send correct message") {
        int before_call = socket->send_text_calls;
        nlohmann::json expected = fix.json_to_send;
        expected["type"] = fix.type;

        fix.socket_notifier.notifyMember(fix.user_id, fix.json_to_send, fix.type);
        REQUIRE(socket->send_text_calls == before_call + 1);
        REQUIRE(socket->last_sended_text == expected.dump());
    }
}
