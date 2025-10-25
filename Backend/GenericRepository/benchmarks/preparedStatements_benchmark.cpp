#include "benchmark/benchmark.h"
#include "../SQLiteDataBase.h"
#include "../GenericReposiroty.h"

static void PrepareQuery(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    QString sql = "SELECT * FROM %1 WHERE id = ?";
    for (auto _ : state) {
        auto treadDb =  db.getThreadDatabase();
        QSqlQuery q(treadDb);
        q.prepare(QString(sql).arg("messages"));
        benchmark::DoNotOptimize(q);
    }
}

static void PrepareQueryWithCache(benchmark::State& state) {
    SQLiteDatabase db;
    GenericRepository rep(db);
    QString sql = "SELECT * FROM %1 WHERE id = ?";
    for (auto _ : state) {
        auto& q = rep.getPreparedQuery("key", sql);
        benchmark::DoNotOptimize(q);
    }
}

BENCHMARK(PrepareQuery)->Iterations(100);
BENCHMARK(PrepareQueryWithCache)->Iterations(100);

//cmake .. -DCMAKE_BUILD_TYPE=Release
//cmake --build . --target benchmarks

