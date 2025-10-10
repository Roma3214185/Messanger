cmake_minimum_required(VERSION 3.16...4.0)

cmake_policy(VERSION 3.5)  # або 3.5

project(ApGateWay)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)



find_package(Threads REQUIRED)
include(FetchContent)

# Завантажуємо cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)


FetchContent_MakeAvailable(httplib )

# Виконуваний файл
add_executable(ApiGateWay
    main.cpp
    src/AuthVerifier.h
    src/proxyclient.cpp
    src/proxyclient.h
    src/ratelimiter.h
)

# Include headers
target_include_directories(ApiGateWay PRIVATE
    ${httplib_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include  # якщо є локальні заголовки
)


target_include_directories(ApiGateWay PRIVATE
    ${CMAKE_SOURCE_DIR}/external/json/include
    ${CMAKE_SOURCE_DIR}/external/crow/include
)


# Link pthread, Crow (якщо потрібен)
# target_link_libraries(api-ApigateWay PRIVATE ${CMAKE_THREAD_LIBS_INIT} Crow::Crow)
