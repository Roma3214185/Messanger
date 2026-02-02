#include "GatewayController.h"

#include <fmt/format.h>
#include <uuid/uuid.h>

#include <chrono>
#include <cstdlib>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "config/Routes.h"
#include "config/ports.h"
#include "interfaces/ICacheService.h"
#include "interfaces/IMetrics.h"
#include "interfaces/IRabitMQClient.h"
#include "interfaces/IThreadPool.h"
#include "middlewares/Middlewares.h"
#include "websocketbridge.h"
#include "proxyclient.h"
#include "entities/RequestDTO.h"

#include "gatewayserver.h"

using std::string;

namespace {

inline long long getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
    .count();
}

inline std::string generateRequestID() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char str[37];
    uuid_unparse(uuid, str);
    return {str};
}

std::string getContentType(const crow::request &req) {
    string content_type = req.get_header_value("content-type");
    if (content_type.empty()) content_type = "application/json";
    return content_type;
}

void sendResponse(crow::response &res, int res_code, const std::string &message) {
    res.code = res_code;
    res.write(message);
    res.end();
}

}  // namespace

GatewayController::GatewayController(IClient *client, ICacheService *cache, IThreadPool *pool, IEventBus *queue)
    : proxy_(client), cache_(cache), pool_(pool), queue_(queue) { }

void GatewayController::handleProxyRequest(const crow::request &req, crow::response &res, const int port,
                                           const std::string &path) {
    RequestDTO request_info = utils::getDTO(req, path);
    auto result = proxy_.forward(request_info, port);
    sendResponse(res, result.first, result.second);
}

void GatewayController::handlePostRequest(
    const crow::request &req,  // todo: make handlers and unordered_map<request, handler>
    crow::response &res, const int port, const std::string &path) {
    RequestDTO request_info =  utils::getDTO(req, path);
    cache_->set("request:" + request_info.request_id, "{ \"status\": \"queued\" }", std::chrono::seconds(30));
    auto json = nlohmann::json(request_info);
    json["port"] = port;

    const PublishRequest publish_request{// todo: make PublishRequest and RequestDTO immutable
                                         .exchange = Config::Routes::exchange,
                                         .routing_key = Config::Routes::sendRequest,
                                         .message = json.dump(),
                                         .exchange_type = "direct"};

    queue_->publish(publish_request);
    nlohmann::json responce;
    responce["status"] = "queued";
    responce["request_id"] = request_info.request_id;
    sendResponse(res, Config::StatusCodes::accepted, responce.dump());
}

void GatewayController::handleRequestRoute(crow::response& res, std::string task_id) {
    LOG_INFO("Request id = {}", task_id);
    std::optional<std::string> status = cache_->get("request:" + task_id);
    nlohmann::json responce;
    std::optional<std::string> id = cache_->get("request_id:" + task_id);
    std::optional<std::string> body = cache_->get("request_body:" + task_id);

    if (!status || !id || !body) {
        responce["status"] = "not_found";
        if (!status) LOG_INFO("Not found status");
        if (!id) LOG_INFO("Not found id");
        if (!body) LOG_INFO("Not found body");
        sendResponse(res, 404, responce.dump());
    } else {
        LOG_INFO("Status found, return {}", *body);
        sendResponse(res, stoi(*id), *body);
    }
}

void GatewayController::subscribeOnNewRequest() {
    SubscribeRequest subscribe_request{.queue = Config::Routes::sendRequest,
                                       .exchange = Config::Routes::exchange,
                                       .routing_key = Config::Routes::sendRequest,
                                       .exchange_type = "direct"};

    queue_->subscribe(subscribe_request, [this](const std::string &event, const std::string &payload) {
        LOG_INFO("I in subscribe with event {} and payload {}", event, payload);
        auto request_info_port = [payload]() -> std::optional<std::pair<RequestDTO, int>> {
            try {
                auto json = nlohmann::json::parse(payload);
                RequestDTO dto = json.get<RequestDTO>();
                const int port = json.at("port").get<int>();
                return std::make_pair(dto, port);
            } catch (const std::exception &e) {
                LOG_ERROR("Can't parse RequestDTO from payload {}: {}", payload, e.what());
                return std::nullopt;
            } catch (...) {
                LOG_ERROR("Unknown error while parsing RequestDTO from payload {}", payload);
                return std::nullopt;
            }
        }();

        if (!request_info_port) return;
        auto [request_info, port] = *request_info_port;
        auto result = proxy_.forward(request_info, port);
        LOG_INFO(
            "Finished result in queue_->subscribe, request_info->request_id = "
            "{}, status_code = "
            "{}, body = {}",
            request_info.request_id, std::to_string(result.first), result.second.substr(0, result.second.length()));

        cache_->set("request:" + request_info.request_id, "{\"status\":\"finished\"}", std::chrono::seconds(30));
        cache_->set("request_id:" + request_info.request_id, std::to_string(result.first), std::chrono::seconds(30));
        cache_->set("request_body:" + request_info.request_id,
                    result.second.substr(0, result.second.length() - 1) + ",\"status\":\"finished\"}",
                    std::chrono::seconds(30));  // todo: fully refactor server responce JsonObject,
        //  return ["error"], ["body"], maybe ["code"]
    });
}
