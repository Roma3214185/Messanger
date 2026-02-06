#ifndef MOCKUSERSOCKETREPOSITORY_H
#define MOCKUSERSOCKETREPOSITORY_H

#include "interfaces/IUserNetworkManager.h"
#include "notificationservice/SocketRepository.h"

class MockUserSocketRepository : public IUserSocketRepository {
public:
    UserSocketsMap user_sockets_;

    void saveConnections(UserId user_id, SocketPtr socket) override;

    SocketPtr getUserSocket(UserId user_id) override;

    bool userOnline(UserId user_id) override;
};

#endif // MOCKUSERSOCKETREPOSITORY_H
