#include <catch2/catch_all.hpp>
#include "notificationservice/SocketRepository.h"
#include "mocks/notificationservice/MockSocket.h"

TEST_CASE("Test Empty socket repository") {
    SocketRepository repository;

    SECTION("Empty repository expected getUserSocket == nullptr") {
        REQUIRE_FALSE(repository.getUserSocket(12));
    }

    SECTION("Empty repository expected userOffline == false") {
        REQUIRE_FALSE(repository.userOnline(12));
    }
}

TEST_CASE("Test socket repository with added user connection") {
    SocketRepository repository;
    const int user_id = 123;
    auto socket = std::make_shared<MockSocket>();
    repository.saveConnections(user_id, socket);

    SECTION("Repository with saved connection expected getUserSocket == added_socket for this id") {
        REQUIRE(repository.getUserSocket(user_id) == socket);
    }

    SECTION("Repository with saved connection expected getUserSocket == added_socket for this another_id") {
        int another_id = 41;
        REQUIRE(repository.getUserSocket(another_id) != socket);
    }

    SECTION("Repository with saved connection expected after deletion getUserSocket == nullptr for this id") {
        repository.deleteConnection(socket);
        REQUIRE_FALSE(repository.getUserSocket(user_id));
    }

    SECTION("Repository with saved connection expected after deletion another one socket getUserSocket == saved socket for this id") {
        auto another_socket = std::make_shared<MockSocket>();

        repository.deleteConnection(another_socket);

        REQUIRE(repository.getUserSocket(user_id) == socket);
    }

    SECTION("Repository with saved connection expected user is online") {
        REQUIRE(repository.userOnline(user_id));
    }

    SECTION("Repository with saved connection after deleting this expected user is offline") {
        repository.deleteConnection(socket);

        REQUIRE_FALSE(repository.userOnline(user_id));
    }

    SECTION("Repository with no crow::sockets expected find socket for connection return nullptr") {
        REQUIRE_FALSE(repository.findSocket(nullptr));
    }
}
