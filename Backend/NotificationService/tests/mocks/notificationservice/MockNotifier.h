#ifndef MOCKNOTIFIER_H
#define MOCKNOTIFIER_H

#include "notificationservice/SocketNotifier.h"

class MockNotifier : public INotifier {
public:
    std::vector<long long> last_user_ids;
    std::vector<nlohmann::json> last_json_message;
    std::vector<std::string> last_types;
    int calls_notifyMember = 0;

    bool notifyMember(long long user_id, nlohmann::json json_message, std::string type) override;
};

#endif // MOCKNOTIFIER_H
