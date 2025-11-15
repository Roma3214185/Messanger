#include <catch2/catch_all.hpp>

#include "messageservice/controller.h"
#include "mocks/MockRabitMQClient.h"
#include "GenericRepository.h"
#include "interfaces/IDataBase.h"
#include "interfaces/ISqlExecutor.h"
#include "interfaces/ICacheService.h"
#include "messageservice/managers/MessageManager.h"

#include "mocks/MockConfigProvider.h"
#include "mocks/MockRabitMQClient.h"

class MockDatabase : public IDataBase{

};

class MockSqlExecutor : public ISqlExecutor{
  public:
    bool execute(const QString& sql, QSqlQuery& outQuery, const QList<QVariant>& values = {}) override {

    }

    std::optional<long long> executeReturningId(const QString&         sql,
                                                        QSqlQuery&             outQuery,
                                                        const QList<QVariant>& values = {}) override {
    }
};

class MockCacheService : public ICacheService{
  public:
    void                          clearPrefix(const std::string& key) override {

    }

    void                          clearCache() override {

    }
    void                          remove(const std::string& key) override {

    }
    void                          incr(const std::string& key)   override {

    }
    bool                          exists(const std::string& key) override {

    }
    std::optional<nlohmann::json> get(const std::string& key)    override {

    }
    void                          set(const std::string&        key,
             const nlohmann::json&     value,
             std::chrono::milliseconds ttl = std::chrono::hours(24)) override {

    }
    void                          setPipelines(const std::vector<std::string>&    keys,
                              const std::vector<nlohmann::json>& results,
                              std::chrono::minutes               ttl = std::chrono::minutes(30)) override {

    }
};

class TestController : public Controller {
  public:
    int call_save_message = 0;
    using Controller::subscribeSaveMessage;
    using Controller::Controller;
    void handleSaveMessage(const std::string& payload) {
      ++call_save_message;
    }
};

TEST_CASE("Test cotroller works with rabitMQ") {
  MockRabitMQClient rabiq_client;
  MockDatabase db;
  MockSqlExecutor executor;
  MockCacheService cache;

  Routes mock_routes;
  mock_routes.messageSaved       = "test_message_saved";
  mock_routes.saveMessage        = "test_save_message";
  mock_routes.saveMessageStatus  = "test_save_message_status";
  mock_routes.notificationQueue  = "test_notification_service_queue";
  mock_routes.exchange           = "test_app.events";
  mock_routes.messageQueue       = "test_message_service_queue";
  mock_routes.messageStatusSaved = "test_message_status_saved";

  MockConfigProvider provider;
  provider.mock_routes = mock_routes;


  GenericRepository rep(db, executor, cache);
  MessageManager manager(&rep);
  TestController controller(&rabiq_client, &manager, &provider);


  SECTION("Subscrive on message to save expected valid data") {
    int before = rabiq_client.subscribe_cnt;
    controller.subscribeSaveMessage();

    REQUIRE(rabiq_client.subscribe_cnt == before + 1);
  }
}
