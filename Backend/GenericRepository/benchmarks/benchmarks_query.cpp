#include "benchmark/benchmark.h"
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../GenericRepository/Query.h"
#include <QCoreApplication>

static void QueryWithoutCache(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    auto q = rep.query<Message>().filter("id", 4).filter("receiver_id", 5);
    for (auto _ : state) {
        auto results = q.execute();
        benchmark::DoNotOptimize(results);
    }
}

static void QueryWithCache(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    auto q = rep.query<Message>().filter("id", 4).filter("receiver_id", 5);
    for (auto _ : state) {
        auto results = q.executeWithoutCache();
        benchmark::DoNotOptimize(results);
    }
}

static void QueryWithCacheAsync(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    auto q = rep.query<Message>().filter("id", 4).filter("receiver_id", 5);
    for (auto _ : state) {
        auto future = q.executeAsync();
        auto results = future.get();
        benchmark::DoNotOptimize(results);
    }
}

static void QueryWithoutCacheAsync(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    auto q = rep.query<Message>().filter("id", 4).filter("receiver_id", 5);
    for (auto _ : state) {
        auto future = q.executeWithoutCacheAsync();
        auto results = future.get();
        benchmark::DoNotOptimize(results);
    }
}

BENCHMARK(QueryWithoutCache)->Iterations(100);
BENCHMARK(QueryWithCache)->Iterations(100);
BENCHMARK(QueryWithCacheAsync)->Iterations(100);
BENCHMARK(QueryWithoutCacheAsync)->Iterations(100);

//cmake .. -DCMAKE_BUILD_TYPE=Release
//cmake --build . --target benchmarks
