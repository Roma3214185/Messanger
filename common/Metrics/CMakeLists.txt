cmake_minimum_required(VERSION 3.22)
project(Metrics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

add_library(Metrics STATIC src/tracy.cpp)

target_include_directories(Metrics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Metrics PUBLIC
    fmt::fmt
    spdlog::spdlog

    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_common.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_trace.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_resources.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_proto.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_proto_grpc.dylib"

    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_common.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_elasticsearch_logs.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_in_memory_metric.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_in_memory.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_ostream_logs_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_ostream_logs.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_ostream_metrics_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_ostream_metrics.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_ostream_span_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_ostream_span.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_builder_utils.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc_client.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc_log_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc_log.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc_metric_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc_metrics.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http_client.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http_log_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http_log.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http_metric_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http_metric.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_http.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_prometheus_builder.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_prometheus.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_http_client_curl.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_logs.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_metrics.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_otlp_recordable.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_proto_grpc.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_proto.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_resources.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_trace.dylib"
    # "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_version.dylib"
)

set_target_properties(Metrics PROPERTIES
    INSTALL_RPATH "/opt/homebrew/opt/opentelemetry-cpp/lib"
)
