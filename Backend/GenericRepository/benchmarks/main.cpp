#include "benchmark/benchmark.h"
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../GenericRepository/Query.h"
#include <QCoreApplication>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    // SQLiteDatabase db;
    // globalDb = &db;

    auto null_logger = spdlog::stderr_color_mt("null");
    null_logger->set_level(spdlog::level::off);
    spdlog::set_default_logger(null_logger);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

//cmake .. -DCMAKE_BUILD_TYPE=Release
//cmake --build . --target benchmarks


// #include "ThreadPool.h"
// #include "benchmark/benchmark.h"
// #include "GenericRepository.h"

// static void SyncDBQueries(benchmark::State& state) {
//     SQLiteDatabase db;
//     GenericRepository repo(db);

//     for (auto _ : state) {
//         repo.find<Message>("SELECT * FROM messages WHERE id < 100;");
//     }
// }

// static void AsyncDBQueries(benchmark::State& state) {
//     SQLiteDatabase db;
//     ThreadPool pool(4);
//     GenericRepository repo(db, pool);

//     for (auto _ : state) {
//         auto f1 = repo.findAsync<Message>("SELECT * FROM messages WHERE id < 100;");
//         f1.get();
//     }
// }

// BENCHMARK(SyncDBQueries);
// BENCHMARK(AsyncDBQueries);
// BENCHMARK_MAIN();
