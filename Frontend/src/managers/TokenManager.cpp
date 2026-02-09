#include "managers/TokenManager.h"
#include "Debug_profiling.h"

void TokenManager::setData(const QString &token, long long current_id) {
    DBC_REQUIRE(!token.isEmpty());
    DBC_REQUIRE(current_id > 0);
    token_ = token;
    current_user_id_ = current_id;
}

QString TokenManager::getToken() noexcept(false) {
    DBC_REQUIRE(token_ != std::nullopt);
    if (!token_.has_value()) {
        throw std::runtime_error("token is empty");
    }
    return token_.value();
}

long long TokenManager::getCurrentUserId() noexcept(false) {
    DBC_REQUIRE(current_user_id_ != std::nullopt);
    if (!current_user_id_.has_value()) {
        throw std::runtime_error("current_id_ is empty");
    }
    return current_user_id_.value();
}

void TokenManager::resetData() {
    token_.reset();
    current_user_id_.reset();
}
