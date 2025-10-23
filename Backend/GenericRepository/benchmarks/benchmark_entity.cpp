#include "benchmark/benchmark.h"
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../GenericRepository/Query.h"
#include <QCoreApplication>
#include "../ThreadPool.h"

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

static void EntityWithCacheAsync(benchmark::State& state) {
    ThreadPool pool(4);
    SQLiteDatabase db;
    GenericRepository rep(db, &pool);
    for (auto _ : state) {
        auto future = rep.findOneAsync<Message>(4);
        auto results = future.get();
        benchmark::DoNotOptimize(results);
    }
}

static void EntityWithoutCacheAsync(benchmark::State& state) {
    ThreadPool pool(4);
    SQLiteDatabase db;
    GenericRepository rep(db, &pool);
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

//cmake .. -DCMAKE_BUILD_TYPE=Release
//cmake --build . --target benchmarks



// std::vector<T> execute() const {
//     QString sql = buildSelectQuery();
//     auto generations = getGenerations();
//     std::size_t generationHash = hashGenerations(generations);
//     std::size_t paramsHash = hashParams(values);

//     std::string cacheKey = createCacheKey(sql, generationHash, paramsHash);

//     if (auto cached = cache.get<std::vector<T>>(cacheKey)) {
//         LOG_INFO("[QueryCache] HIT for key '{}'", cacheKey);
//         return *cached;
//     }

//     LOG_INFO("[QueryCache] NOT HITTED for key '{}'", cacheKey);

//     QSqlDatabase threadDb = db.getThreadDatabase(); // <-- use value, not ref
//     QSqlQuery query(threadDb);
//     query.prepare(sql);
//     for (int i = 0; i < values.size(); ++i)
//         query.bindValue(i, values[i]);

//     if (!query.exec()) {
//         LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
//         return {};
//     }

//     auto results = std::vector<T>{};
//     auto meta = Reflection<T>::meta();

//     while (query.next()){
//         T entity = buildEntity(query, meta);
//         results.push_back(entity);
//         saveEntityInCache(entity);
//     }

//     LOG_INFO("Result size is '{}' is setted in cashe for key '{}'", results.size(), cacheKey);
//     cache.set(cacheKey, results, std::chrono::hours(24));
//     return results;
// }
