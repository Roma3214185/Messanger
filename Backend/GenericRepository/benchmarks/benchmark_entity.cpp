#include "benchmark/benchmark.h"
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../GenericRepository/Query.h"
#include <QCoreApplication>

//static SQLiteDatabase* globalDb = nullptr;


static void EntityWithoutCache(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    for (auto _ : state) {
        auto results = rep.findOne<Message>(4);
        benchmark::DoNotOptimize(results);
    }
}

static void EntityWithCache(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    for (auto _ : state) {
        auto results = rep.findOneWithOutCache<Message>(4);
        benchmark::DoNotOptimize(results);
    }
}


BENCHMARK(EntityWithoutCache)->Iterations(100);
BENCHMARK(EntityWithCache)->Iterations(100);
