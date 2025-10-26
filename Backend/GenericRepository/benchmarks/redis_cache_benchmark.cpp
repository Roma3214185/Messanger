#include <benchmark/benchmark.h>
#include "RedisCache.h"
#include "MessageService/Headers/Message.h"

static void BM_SaveEntityIndividually(benchmark::State& state) {
    RedisCache& cache = RedisCache::instance();
    std::vector<Message> entities;

    for (int i = 0; i < state.range(0); ++i) {
        entities.push_back(Message{
            .id = i,
            .text = "Name_" + std::to_string(i),
            .sender_id = i + 12
        });
    }

    for (auto _ : state) {
        for (const auto& e : entities) {
            cache.saveEntity(e, "messages");
        }
    }
}

static void BM_SaveEntityPipeline(benchmark::State& state) {
    RedisCache& cache = RedisCache::instance();
    std::vector<Message> entities;

    for (int i = 0; i < state.range(0); ++i) {
        entities.push_back(Message{
            .id = i,
            .text = "Name_" + std::to_string(i),
            .sender_id = i + 12
        });
    }

    for (auto _ : state) {
        cache.saveEntities(entities, "messages");
    }
}

BENCHMARK(BM_SaveEntityIndividually)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK(BM_SaveEntityPipeline)->Arg(10)->Arg(100)->Arg(1000);
