#include <algorithm>
#include <atomic>
#include <mutex>
#include <vector>

#include "Persistence/GenericRepository.h"
#include "benchmark/benchmark.h"

static void BM_FindWithCachePercHit(benchmark::State& state) {
  int totalOps = state.range(0);
  int percHit = state.range(1);
  std::atomic<int> id{0};

  for (auto _ : state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    rep.clearCache();

    for (int i = 1; i <= totalOps; i++) {
      if ((i * 100 / totalOps) <= percHit) {
        rep.findOne<Message>(i);
      }
    }

    id.store(0);

    for (int i = 0; i < totalOps; ++i) {
      int currentId = id.fetch_add(1);
      std::optional<Message> result = rep.findOne<Message>(currentId);
      ;
      benchmark::DoNotOptimize(result);
    }
  }
}

static void BM_FindWithCachePercHitAsync(benchmark::State& state) {
  int totalOps = state.range(0);
  int percHit = state.range(1);

  for (auto _ : state) {
    std::vector<int> ids(totalOps);
    for (int i = 0; i < totalOps; ++i) {
      ids[i] = i;
    }

    {
      SQLiteDatabase db;
      GenericRepository rep(db);
      rep.clearCache();
      for (int i = 0; i < totalOps; ++i) {
        if ((i * 100 / totalOps) < percHit) {
          rep.findOne<Message>(ids[i]);
        }
      }
    }

    ThreadPool pool{4};

    for (int i = 0; i < totalOps; ++i) {
      pool.enqueue([id = ids[i]] {
        SQLiteDatabase db;
        GenericRepository rep(db);
        auto result = rep.findOne<Message>(id);
        benchmark::DoNotOptimize(result);
      });
    }

    pool.waitAll();
  }
}

BENCHMARK(BM_FindWithCachePercHit)
    ->Args({100, 100})
    ->Args({100, 80})
    ->Args({100, 60})
    ->Args({100, 40})
    ->Args({100, 20})
    ->Args({100, 0});

BENCHMARK(BM_FindWithCachePercHitAsync)
    ->Args({100, 100})
    ->Args({100, 80})
    ->Args({100, 60})
    ->Args({100, 40})
    ->Args({100, 20})
    ->Args({100, 0});
