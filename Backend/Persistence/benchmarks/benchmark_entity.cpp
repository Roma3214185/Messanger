#include <QCoreApplication>

#include "Persistence/ThreadPool.h"
#include "Persistence/GenericRepository.h"
#include "Persistence/Query.h"
#include "benchmark/benchmark.h"
#include "SqlExecutor.h"

static void EntityWithoutCache(benchmark::State& state) {
  SQLiteDatabase db;
  SqlExecutor executor(db);
  GenericRepository rep(executor, RedisCache::instance());
  for (auto _ : state) {
    auto results = rep.findOne<Message>(4);
    benchmark::DoNotOptimize(results);
  }
}

static void EntityWithCache(benchmark::State& state) {
  SQLiteDatabase db;
  SqlExecutor executor(db);
  GenericRepository rep(executor, RedisCache::instance());
  for (auto _ : state) {
    auto results = rep.findOneWithOutCache<Message>(4);
    benchmark::DoNotOptimize(results);
  }
}

static void EntityWithCacheAsync(benchmark::State& state) {
  ThreadPool pool(4);
  SQLiteDatabase db;
  SqlExecutor executor(db);
  GenericRepository rep(executor, RedisCache::instance(), &pool);
  for (auto _ : state) {
    auto future = rep.findOneAsync<Message>(4);
    auto results = future.get();
    benchmark::DoNotOptimize(results);
  }
}

static void EntityWithoutCacheAsync(benchmark::State& state) {
  ThreadPool pool(4);
  SQLiteDatabase db;
  SqlExecutor executor(db);
  GenericRepository rep(executor, RedisCache::instance(), &pool);
  for (auto _ : state) {
    auto future = rep.findOneWithOutCacheAsync<Message>(4);
    auto results = future.get();
    benchmark::DoNotOptimize(results);
  }
}

BENCHMARK(EntityWithoutCache)->Iterations(100);
BENCHMARK(EntityWithCache)->Iterations(100);
BENCHMARK(EntityWithCacheAsync)->Iterations(100);
BENCHMARK(EntityWithoutCacheAsync)->Iterations(100);

// cmake .. -DCMAKE_BUILD_TYPE=Release
// cmake --build . --target benchmarks
