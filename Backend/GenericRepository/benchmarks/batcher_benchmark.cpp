#include "benchmark/benchmark.h"
#include "GenericReposiroty.h"
#include "ThreadPool.h"

static void individualSaving(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    for (auto _ : state) {
        for(int i = 0; i < state.range(0); i++){
            Message msg;
            rep.save(msg);
            benchmark::DoNotOptimize(msg);
        }
    }
}

static void batcherSaving(benchmark::State& state) {
    SQLiteDatabase db("bench_conn_global");
    GenericRepository rep(db);

    for (auto _ : state) {
        SaverBatcher<Message> saverBatcher(rep);
        for(int i = 0; i < state.range(0); i++){
            Message msg;
            saverBatcher.saveEntity(msg);
            benchmark::DoNotOptimize(msg);
        }
    }
}

static void individualDeleting(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    for (auto _ : state) {
        for(int i = 0; i < state.range(0); i++){
            Message msg{.id = i};
            rep.deleteEntity(msg);
            benchmark::DoNotOptimize(msg);
        }
    }
}

static void batcherDeleter(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);

    for (auto _ : state) {
        DeleterBatcher<Message> deleterBatcher(rep);
        for(int i = 0; i < state.range(0); i++){
            Message msg{.id = i};
            deleterBatcher.deleteEntity(msg);
            benchmark::DoNotOptimize(msg);
        }
    }
}

BENCHMARK(individualSaving)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(batcherSaving)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(individualDeleting)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(batcherDeleter)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

