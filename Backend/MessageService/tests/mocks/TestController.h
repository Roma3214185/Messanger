#ifndef TESTCONTROLLER_H
#define TESTCONTROLLER_H

#include "messageservice/controller.h"

class TestController : public Controller {
  public:
    int call_save_message = 0;
    int call_save_message_status = 0;
    int call_getMessages = 0;
    GetMessagePack last_getMessages_pack;
    int last_getMessagesUserId;
    std::optional<int> mock_getIdFromToken_ans;

    std::optional<crow::json::wvalue> mock_getMessages_ans;

    std::string last_payload = "";
    std::string last_getIdFromToken_token = "";

    using Controller::subscribeSaveMessage;
    using Controller::subscribeSaveMessageStatus;
    using Controller::Controller;

    void handleSaveMessage(const std::string& payload) override {
      ++call_save_message;
      last_payload = payload;
    }

    void handleSaveMessageStatus(const std::string& payload) override {
      ++call_save_message_status;
      last_payload = payload;
    }
};

#endif // TESTCONTROLLER_H
