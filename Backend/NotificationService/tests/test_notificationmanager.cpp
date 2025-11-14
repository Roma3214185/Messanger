#include <catch2/catch_all.hpp>
#include "managers/notificationmanager.h"
#include "mocks/MockRabitMQClient.h"
#include "managers/SocketManager.h"
#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "NetworkFacade.h"

class MockNetworkManager
    : public IChatNetworkManager
    , public IUserNetworkManager
    , public IMessageNetworkManager {
  public:

};

TEST_CASE("First test") {
  MockRabitMQClient mock_rabit_client;
  SocketsManager socket_manager;
  MockNetworkManager network_manager;
  NetworkFacade network_reposiroty = NetworkFactory::create(&network_manager);

  NotificationManager notificationmanager(&mock_rabit_client, socket_manager, network_reposiroty);


}
