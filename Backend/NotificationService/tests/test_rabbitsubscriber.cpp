#include <catch2/catch_all.hpp>
#include "notificationservice/ISubscriber.h"
#include "mocks/MockRabitMQClient.h"
#include "notificationservice/managers/NotificationOrchestrator.h"
#include "notificationservice/IPublisher.h"
#include "notificationservice/SocketNotifier.h"
#include "NetworkFacade.h"
#include "config/Routes.h"

class RabitMQSubscriberTester : public RabbitNotificationSubscriber {
public:
    using RabbitNotificationSubscriber::RabbitNotificationSubscriber;
    using RabbitNotificationSubscriber::subscribeAll;
    using RabbitNotificationSubscriber::subscribeMessageDeleted;
    using RabbitNotificationSubscriber::subscribeMessageReactionDeleted;
    using RabbitNotificationSubscriber::subscribeMessageReactionSaved;
    using RabbitNotificationSubscriber::subscribeMessageSaved;
    using RabbitNotificationSubscriber::subscribeMessageStatusSaved;
};

class MockUserNetworkManager : public IUserNetworkManager {
public:
    std::optional<User> responce_getUserById;
    long long last_other_user_id = -1;
    std::optional<User> getUserById(long long other_user_id) override {
        last_other_user_id = other_user_id;
        return responce_getUserById;
    }
};

class MockMessageNetworkManager : public IMessageNetworkManager {
  public:
    long long last_message_id = -1;
    long long last_reaction_id = -1;
    std::optional<long long> responce_getChatIdOfMessage;
    std::optional<ReactionInfo> responce_getReaction;


    std::optional<long long> getChatIdOfMessage(long long message_id) override {
        last_message_id = message_id;
        return responce_getChatIdOfMessage;
    }

    std::optional<ReactionInfo> getReaction(long long reaction_id) override {
        last_reaction_id = reaction_id;
        return responce_getReaction;
    }
};

class MockChatNetworkManager : public IChatNetworkManager {
    public:
    std::vector<UserId> responce_getMembersOfChat;
    long long last_chat_id = -1;

    std::vector<UserId> getMembersOfChat(long long chat_id) override {
        last_chat_id = chat_id;
        return responce_getMembersOfChat;
    }
};

class MockFacade : public INetworkFacade {
public:
    IUserNetworkManager& users() override {
        return users_manager;
    }
    IMessageNetworkManager& messages() override {
        return messages_manager;
    }
    IChatNetworkManager& chats() override {
        return chats_manager;
    }

    MockUserNetworkManager users_manager;
    MockMessageNetworkManager messages_manager;
    MockChatNetworkManager chats_manager;
};

class MockPublisher : public IPublisher {
public:
    int calls_saveMessageStatus = 0;
    std::vector<MessageStatus> messages_status_to_save;
    void saveMessageStatus(MessageStatus &status) override {
        ++calls_saveMessageStatus;
        messages_status_to_save.push_back(status);
    }

    int calls_saveReaction = 0;
    std::vector<Reaction> reactions_to_save;
    void saveReaction(const Reaction &reaction) override {
        ++calls_saveMessage;
        reactions_to_save.push_back(reaction);
    }

    int calls_deleteReaction = 0;
    std::vector<Reaction> reactions_to_delete;
    void deleteReaction(const Reaction &reaction) override {
        ++calls_deleteReaction;
        reactions_to_delete.push_back(reaction);
    }

    int calls_saveMessage = 0;
    std::vector<Message> messages_to_save;
    void saveMessage(const Message &message) override {
        ++calls_saveMessage;
        messages_to_save.push_back(message);
    }

    int calls_deleteMessageStatus = 0;
    std::vector<MessageStatus> messages_status_to_delete;
    void deleteMessageStatus(const MessageStatus &message) override {
        ++calls_deleteMessageStatus;
        messages_status_to_delete.push_back(message);
    }
};

class MockNotifier : public INotifier {
public:
    std::vector<long long> last_user_ids;
    std::vector<nlohmann::json> last_json_message;
    std::vector<std::string> last_types;
    int calls_notifyMember = 0;

    bool notifyMember(long long user_id, nlohmann::json json_message, std::string type) override {
        last_user_ids.push_back(user_id);
        last_json_message.push_back(json_message);
        last_types.push_back(type);
        ++calls_notifyMember;
        return true;
    }
};

struct NotificationOrchestratorTestFixture {
    MockRabitMQClient queue;
    MockFacade facade;
    MockPublisher publisher;
    MockNotifier notifier;

    NotificationOrchestrator orchestor;
    RabitMQSubscriberTester rabbit_notification_subscriber;

    NotificationOrchestratorTestFixture()
        : orchestor(&facade, &publisher, &notifier)
        , rabbit_notification_subscriber(&queue, &orchestor) {}
};


TEST_CASE("Test RabbitNotificationSubscriber subscribeAll") {
    NotificationOrchestratorTestFixture fix;
    WHEN("Rabbit_notification_subscriber calls subscribeAll() function") {
        fix.rabbit_notification_subscriber.subscribeAll();

        THEN("Class calls 5 specific fucntions") {
            REQUIRE(fix.queue.subscribe_cnt == 5);
        }
    }
}

TEST_CASE("Test RabbitNotificationSubscriber subscribeMessageSaved") {
    NotificationOrchestratorTestFixture fix;
    int chat_id = 121;
    std::vector<long long> members_of_chat{12, 41};
    Message message_to_save;
    message_to_save.chat_id = chat_id;
    fix.facade.chats_manager.responce_getMembersOfChat = members_of_chat;

    WHEN("Rabbit_notification_subscriber calls subscribeMessageSaved() function") {
        fix.rabbit_notification_subscriber.subscribeMessageSaved();

        THEN("Class calls specific fucntions") {
            REQUIRE(fix.queue.subscribe_cnt == 1);
        }

        AND_THEN("Class form correct Subscribe request") {
            auto last_request = fix.queue.last_subscribe_request;

            CHECK(last_request.exchange == Config::Routes::exchange);
            CHECK(last_request.exchange_type == Config::Routes::exchangeType);
            CHECK(last_request.queue == Config::Routes::messageSavedQueue);
            CHECK(last_request.routing_key == Config::Routes::messageSaved);
        }

        AND_THEN("Queue calls callback") {
            fix.queue.callLastCallback(nlohmann::json(message_to_save).dump());

            THEN("Suscriber calls orchestor") {

                AND_THEN("Orchestor fetch chat members for chat message_to_save.chat_id") {
                    REQUIRE(fix.facade.chats_manager.last_chat_id == chat_id);
                }

                AND_THEN("Publisher get calls 2 times to save message statuses") {
                    REQUIRE(fix.publisher.calls_saveMessageStatus == 2);
                    auto messages_to_save = fix.publisher.messages_status_to_save;
                    REQUIRE(messages_to_save.size() == 2);
                    REQUIRE(messages_to_save[0].receiver_id == members_of_chat[0]);
                    REQUIRE(messages_to_save[1].receiver_id == members_of_chat[1]);
                }


                AND_WHEN("Notifier get calls 2 times to notify saved messages") {
                    REQUIRE(fix.notifier.calls_notifyMember == 2);
                    REQUIRE(fix.notifier.last_user_ids == members_of_chat);
                }
            }

        }
    }
}

TEST_CASE("Test RabbitNotificationSubscriber subscribeMessageDeleted") {
    NotificationOrchestratorTestFixture fix;
    int chat_id = 124;
    std::vector<long long> members_of_chat{12, 41, 3};
    Message message_to_delete;
    message_to_delete.chat_id = chat_id;
    fix.facade.chats_manager.responce_getMembersOfChat = members_of_chat;

    WHEN("Rabbit_notification_subscriber calls subscribeMessageSaved() function") {
        fix.rabbit_notification_subscriber.subscribeMessageDeleted();

        THEN("Class calls specific fucntions") {
            REQUIRE(fix.queue.subscribe_cnt == 1);
        }

        AND_THEN("Class form correct Subscribe request") {
            auto last_request = fix.queue.last_subscribe_request;

            CHECK(last_request.exchange == Config::Routes::exchange);
            CHECK(last_request.exchange_type == Config::Routes::exchangeType);
            CHECK(last_request.queue == Config::Routes::messageDeleted);
            CHECK(last_request.routing_key == Config::Routes::messageDeleted);
        }

        AND_THEN("Queue calls callback") {
            fix.queue.callLastCallback(nlohmann::json(message_to_delete).dump());

            THEN("Suscriber calls orchestor") {

                AND_THEN("Orchestor fetch chat members for chat message_to_save.chat_id") {
                    REQUIRE(fix.facade.chats_manager.last_chat_id == chat_id);
                }

                AND_THEN("Publisher get calls 3 times to delete message statuses") {
                    REQUIRE(fix.publisher.calls_deleteMessageStatus == 3);
                    auto messages_statuses_to_delete = fix.publisher.messages_status_to_delete;
                    REQUIRE(messages_statuses_to_delete.size() == 3);
                    REQUIRE(messages_statuses_to_delete[0].receiver_id == members_of_chat[0]);
                    REQUIRE(messages_statuses_to_delete[0].message_id == message_to_delete.id);
                    REQUIRE(messages_statuses_to_delete[1].receiver_id == members_of_chat[1]);
                    REQUIRE(messages_statuses_to_delete[1].message_id == message_to_delete.id);
                    REQUIRE(messages_statuses_to_delete[1].receiver_id == members_of_chat[1]);
                    REQUIRE(messages_statuses_to_delete[2].message_id == message_to_delete.id);
                }

                AND_WHEN("Notifier get calls 3 times to notify users about deleted message") {
                    REQUIRE(fix.notifier.calls_notifyMember == 3);
                    REQUIRE(fix.notifier.last_user_ids == members_of_chat);
                }
            }

        }
    }
}

TEST_CASE("Test RabbitNotificationSubscriber subscribeMessageReactionSaved") {
    NotificationOrchestratorTestFixture fix;
    Reaction reaction_to_save;
    reaction_to_save.message_id = 121;
    reaction_to_save.reaction_id = 11;
    reaction_to_save.receiver_id = 1456;

    int chat_id = 124;
    std::vector<long long> members_of_chat{112, 411, 34};
    fix.facade.chats_manager.responce_getMembersOfChat = members_of_chat;

    WHEN("Rabbit_notification_subscriber calls subscribeMessageReactionSaved() function") {
        fix.rabbit_notification_subscriber.subscribeMessageReactionSaved();

        THEN("Class calls specific fucntions") {
            REQUIRE(fix.queue.subscribe_cnt == 1);
        }

        AND_THEN("Class form correct Subscribe request") {
            auto last_request = fix.queue.last_subscribe_request;

            CHECK(last_request.exchange == Config::Routes::exchange);
            CHECK(last_request.exchange_type == Config::Routes::exchangeType);
            CHECK(last_request.queue == Config::Routes::messageReactionSaved);
            CHECK(last_request.routing_key == Config::Routes::messageReactionSaved);
        }

        WHEN("Queue calls callback with not finded chat_id") {
            fix.facade.messages_manager.responce_getChatIdOfMessage = std::nullopt;
            fix.queue.callLastCallback(nlohmann::json(reaction_to_save).dump());

            THEN("Notificator get 0 calls") {
                  REQUIRE(fix.notifier.calls_notifyMember == 0);
            }
        }

        WHEN("Queue calls callback with finded chat_id") {
            fix.facade.messages_manager.responce_getChatIdOfMessage = chat_id;
            fix.queue.callLastCallback(nlohmann::json(reaction_to_save).dump());

            THEN("Orchestor fetch chat members for chat message_to_save.chat_id") {
                REQUIRE(fix.facade.chats_manager.last_chat_id == chat_id);
            }

            AND_WHEN("Notifier get calls 3 times to notify users about saved reaction") {
                REQUIRE(fix.notifier.calls_notifyMember == 3);
                REQUIRE(fix.notifier.last_user_ids == members_of_chat);
            }
        }
    }
}

TEST_CASE("Test RabbitNotificationSubscriber subscribeMessageStatusSaved") {
    NotificationOrchestratorTestFixture fix;
    MessageStatus message_status_to_save;
    message_status_to_save.message_id = 121;
    message_status_to_save.receiver_id = 1211;

    int chat_id = 124;
    std::vector<long long> members_of_chat{112, 411, 34};
    fix.facade.chats_manager.responce_getMembersOfChat = members_of_chat;

    WHEN("Rabbit_notification_subscriber calls subscribeMessageStatusSaved() function") {
        fix.rabbit_notification_subscriber.subscribeMessageStatusSaved();

        THEN("Class calls specific fucntions") {
            REQUIRE(fix.queue.subscribe_cnt == 1);
        }

        AND_THEN("Class form correct Subscribe request") {
            auto last_request = fix.queue.last_subscribe_request;

            CHECK(last_request.exchange == Config::Routes::exchange);
            CHECK(last_request.exchange_type == Config::Routes::exchangeType);
            CHECK(last_request.queue == Config::Routes::messageSavedQueue);
            CHECK(last_request.routing_key == Config::Routes::messageStatusSaved);
        }

        WHEN("Queue calls callback with not MessageStatus") {
            fix.queue.callLastCallback("Bad payload");

            THEN("Notificator get 0 calls") {
                REQUIRE(fix.notifier.calls_notifyMember == 0);
            }
        }

        WHEN("Queue calls callback with not finded chat_id") {
            fix.facade.messages_manager.responce_getChatIdOfMessage = std::nullopt;
            fix.queue.callLastCallback(nlohmann::json(message_status_to_save).dump());

            THEN("Notificator get 0 calls") {
                REQUIRE(fix.notifier.calls_notifyMember == 0);
            }
        }

        WHEN("Queue calls callback with finded chat_id") {
            fix.facade.messages_manager.responce_getChatIdOfMessage = chat_id;
            fix.queue.callLastCallback(nlohmann::json(message_status_to_save).dump());

            THEN("Orchestor fetch chat members for chat message_to_save.chat_id") {
                REQUIRE(fix.facade.chats_manager.last_chat_id == chat_id);
            }

            AND_WHEN("Notifier get calls 3 times to notify users about saved message_reactions") {
                REQUIRE(fix.notifier.calls_notifyMember == 3);
                REQUIRE(fix.notifier.last_user_ids == members_of_chat);
            }
        }
    }
}

TEST_CASE("Test RabbitNotificationSubscriber subscribeMessageReactionDeleted") {
    NotificationOrchestratorTestFixture fix;
    Reaction reaction_to_save;
    reaction_to_save.message_id = 121;
    reaction_to_save.reaction_id = 11;
    reaction_to_save.receiver_id = 1456;

    int chat_id = 124;
    std::vector<long long> members_of_chat{112, 411, 34};
    fix.facade.chats_manager.responce_getMembersOfChat = members_of_chat;

    WHEN("Rabbit_notification_subscriber calls subscribeMessageReactionSaved() function") {
        fix.rabbit_notification_subscriber.subscribeMessageReactionDeleted();

        THEN("Class calls specific fucntions") {
            REQUIRE(fix.queue.subscribe_cnt == 1);
        }

        AND_THEN("Class form correct Subscribe request") {
            auto last_request = fix.queue.last_subscribe_request;

            CHECK(last_request.exchange == Config::Routes::exchange);
            CHECK(last_request.exchange_type == Config::Routes::exchangeType);
            CHECK(last_request.queue == Config::Routes::messageReactionDeleted);
            CHECK(last_request.routing_key == Config::Routes::messageReactionDeleted);
        }

        WHEN("Queue calls callback with not reqction payload") {
            fix.queue.callLastCallback("Bad payload");

            THEN("Notificator get 0 calls") {
                REQUIRE(fix.notifier.calls_notifyMember == 0);
            }
        }

        WHEN("Queue calls callback with not finded chat_id") {
            fix.facade.messages_manager.responce_getChatIdOfMessage = std::nullopt;
            fix.queue.callLastCallback(nlohmann::json(reaction_to_save).dump());

            THEN("Notificator get 0 calls") {
                REQUIRE(fix.notifier.calls_notifyMember == 0);
            }
        }

        WHEN("Queue calls callback with finded chat_id") {
            fix.facade.messages_manager.responce_getChatIdOfMessage = chat_id;
            fix.queue.callLastCallback(nlohmann::json(reaction_to_save).dump());

            THEN("Orchestor fetch chat members for chat reaction.message_id") {
                REQUIRE(fix.facade.chats_manager.last_chat_id == chat_id);
            }

            AND_WHEN("Notifier get calls 3 times to notify users about deleted reaction") {
                REQUIRE(fix.notifier.calls_notifyMember == 3);
                REQUIRE(fix.notifier.last_user_ids == members_of_chat);
            }
        }
    }
}
