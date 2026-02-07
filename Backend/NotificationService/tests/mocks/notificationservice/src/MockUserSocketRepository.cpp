#include "mocks/notificationservice/MockUserSocketRepository.h"

void MockUserSocketRepository::saveConnections(UserId user_id, SocketPtr socket) {
    user_sockets_[user_id] = socket;
}

SocketPtr MockUserSocketRepository::getUserSocket(UserId user_id) {
    return user_sockets_.contains(user_id) ? user_sockets_[user_id] : nullptr;
}

bool MockUserSocketRepository::userOnline(UserId user_id) {
    return true;
}
