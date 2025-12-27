cmake_minimum_required(VERSION 3.22)

if(NOT TARGET Metrics)
    add_library(Metrics STATIC src/tracy.cpp)
    target_compile_features(Metrics PUBLIC cxx_std_20)

    find_package(spdlog REQUIRED)
    find_package(fmt REQUIRED)
    find_package(opentelemetry-cpp CONFIG REQUIRED)


    target_include_directories(Metrics PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(Metrics PUBLIC
        fmt::fmt
        spdlog::spdlog
        opentelemetry-cpp::sdk
        opentelemetry-cpp::metrics
        opentelemetry-cpp::trace
        opentelemetry-cpp::otlp_grpc_exporter
    )

    enable_clang_tidy(Metrics)
    enable_cppcheck(Metrics)
    enable_iwyu(Metrics)

endif()
