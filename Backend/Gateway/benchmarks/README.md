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


### Results of POST request with pattern and std::thread

```bach

```


### Results of POST request with pattern and ThreadPool

```bach

```

### Results of POST request with pattern and RabiqMQ

```bach

```
