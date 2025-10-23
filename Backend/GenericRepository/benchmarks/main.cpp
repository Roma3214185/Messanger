#include "benchmark/benchmark.h"
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../GenericRepository/Query.h"
#include <QCoreApplication>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    auto null_logger = spdlog::stderr_color_mt("null");
    null_logger->set_level(spdlog::level::off);
    spdlog::set_default_logger(null_logger);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

//cmake .. -DCMAKE_BUILD_TYPE=Release
//cmake --build . --target benchmarks
