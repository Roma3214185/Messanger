# Benchmakrs in gateway

Paragraph.

## How Async Request-Responce pattern improve gateway

I will check how many time needed to handle 1000 request(assume that sendind request to another server and waiting for responces 150 ms (this number i checked a couple of times)

### Results of POST request without pattern

Code snippet:
```bach
auto result = proxy_.forward(req, request_info, port); 
return sendResponse(res, result.first, result.second);
```

| Benchmark | Time (ns) | CPU (ns) | Iterations |
|-----------|-----------|-----------|------------|
| BM_SyncRequestResponce/10 | 1546222708 | 295900 | 10 |
| BM_SyncRequestResponce/100  | 1.5476e+10 | 2601000 | 1 |
| BM_SyncRequestResponce/100    | 1.5366e+11  | 82470000  | 1 |


### Results of POST request with pattern and ThreadPool

```bach
RequestDTO request_info = getRequestInfo(req, path);
cache_->set("request:" + request_info.request_id, "{ \"state\": \"queued\" }");

pool_->enqueue([this, request_info, port] {
  auto result = proxy_.forward(request_info, port);  //TODO: rabit mq??
  cache_->set("request:" + request_info.request_id, "{\"state\":\"finished\"}");
  cache_->set("request_id:" + request_info.request_id, std::to_string(result.first));
  cache_->set("request_body:" + request_info.request_id, result.second + "\n{\"state\":\"finished\"}");
});

sendResponse(res, 202, request_info.request_id);
```

| Benchmark | Time (ns) | CPU (ns) | Iterations |
|-----------|-----------|-----------|------------|
| BM_AsyncRequestResponceWithPool/10 | 1547556375 | 332400 | 10 |
| BM_AsyncRequestResponceWithPool/100  | 1.5463e+10 | 3532000 | 1 |
| BM_AsyncRequestResponceWithPool/100    | 1.5392e+11  | 74854000  | 1 |


### Results of POST request with pattern and RabiqMQ

```bach
  RequestDTO request_info = getRequestInfo(req, path, port);
  cache_->set("request:" + request_info.request_id, "{ \"state\": \"queued\" }");

  PublishRequest publish_request{
    .exchange = provider_->routes().exchange,
    .routingKey = provider_->routes().sendRequest,
    .message =  nlohmann::json(request_info).dump(),
    .exchangeType = "direct"
  };

  queue_->publish(publish_request);
  sendResponse(res, 202, request_info.request_id);
}
```

| Benchmark | Time (ns) | CPU (ns) | Iterations |
|-----------|-----------|-----------|------------|
| BM_AsyncRequestResponceWithRabiqMQ/10 | 37261 | 37261 | 19283 |
| BM_AsyncRequestResponceWithRabiqMQ/100  | 363528 | 363528 | 1976 |
| BM_AsyncRequestResponceWithRabiqMQ/100    | 3544509  | 3514560  | 193 |

