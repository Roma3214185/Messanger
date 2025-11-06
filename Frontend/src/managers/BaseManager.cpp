#include "managers/BaseManager.h"

BaseManager::BaseManager(INetworkAccessManager* network_manager,
                         const QUrl& base_url,
                         int timeout_ms,
                         QObject* parent)
    : QObject(parent)
    , network_manager_(network_manager)
    , url_(base_url)
    , timeout_ms_(timeout_ms) {}

BaseManager::~BaseManager() {}
