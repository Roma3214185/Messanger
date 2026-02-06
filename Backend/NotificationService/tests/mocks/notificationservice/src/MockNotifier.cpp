#include "mocks/notificationservice/MockNotifier.h"

bool MockNotifier::notifyMember(long long user_id, nlohmann::json json_message, std::string type) {
    last_user_ids.push_back(user_id);
    last_json_message.push_back(json_message);
    last_types.push_back(type);
    ++calls_notifyMember;
    return true;
}
