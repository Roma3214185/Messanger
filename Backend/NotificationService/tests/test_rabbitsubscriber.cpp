#include <catch2/catch_all.hpp>

#include "notificationservice/ISubscriber.h"
#include "notificationservice/managers/NotificationOrchestrator.h"
#include "NetworkFacade.h"
#include "config/Routes.h"
#include "mocks/MockNetworkManager.h"
#include "mocks/MockPublisher.h"
#include "mocks/notificationservice/MockNotifier.h"
#include "mocks/MockRabitMQClient.h"

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
