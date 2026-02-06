#ifndef MOCKNETWORKMANAGER_H
#define MOCKNETWORKMANAGER_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "NetworkFacade.h"

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

class MockUserNetworkManager : public IUserNetworkManager {
public:
    std::optional<User> responce_getUserById;
    long long last_other_user_id = -1;
    std::optional<User> getUserById(long long other_user_id) override {
        last_other_user_id = other_user_id;
        return responce_getUserById;
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

#endif  // MOCKNETWORKMANAGER_H
