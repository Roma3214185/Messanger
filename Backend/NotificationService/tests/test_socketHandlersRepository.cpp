#include <catch2/catch_all.hpp>
#include "SocketHandlersRepositoty.h"
#include "mocks/notificationservice/MockSocket.h"

class MockHandler : public IMessageHandler{
public:
    int calls_handle = 0;
    crow::json::rvalue last_value;
    std::shared_ptr<ISocket> last_socket;
    void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) override {
        ++calls_handle;
        last_value = message;
        last_socket = socket;
    }
};

TEST_CASE("Test socket handler repository") {
    SocketHandlersRepository repository;
    SocketHandlers handlers;
    std::string type = "load_usages";
    auto handler = std::make_shared<MockHandler>();
    handlers[type] = handler;
    auto socket = std::make_shared<MockSocket>();
    repository.setHandlers(std::move(handlers));

    SECTION("Handle json::wvalue without type expected no call MockHandler.handler()") {
        crow::json::rvalue msg = crow::json::load("{}");
        REQUIRE(msg);

        repository.handle(msg, socket);

        REQUIRE(handler->calls_handle == 0);
    }

    SECTION("Handle json::wvalue with undefined type expected no call MockHandler.handler()") {
        crow::json::wvalue w;
        w["type"] = "ping";
        crow::json::rvalue msg = crow::json::load(w.dump());
        REQUIRE(msg);

        repository.handle(msg, socket);

        REQUIRE(handler->calls_handle == 0);
    }

    SECTION("Handle json::wvalue with setted type expected call MockHandler.handler()") {
        crow::json::wvalue w;
        w["type"] = type;
        crow::json::rvalue msg = crow::json::load(w.dump());
        REQUIRE(msg);

        repository.handle(msg, socket);

        REQUIRE(handler->calls_handle == 1);
        REQUIRE(handler->last_socket == socket);
        REQUIRE(handler->last_value["type"].s() == msg["type"].s());
    }
}
