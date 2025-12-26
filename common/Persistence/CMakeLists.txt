cmake_minimum_required(VERSION 3.22)

if (NOT TARGET Persistence)

    find_package(Qt6 REQUIRED COMPONENTS Core Sql)
    find_package(nlohmann_json REQUIRED)

add_library(Persistence STATIC
    src/GenericRepository.cpp
    src/SQLiteDatabase.cpp
    src/SqlExecutor.cpp
)

target_compile_features(Persistence PUBLIC cxx_std_20)

enable_clang_tidy(Persistence)
enable_cppcheck(Persistence)
enable_iwyu(Persistence)

target_include_directories(Persistence
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/inl
)

target_link_libraries(Persistence
    PUBLIC
        Qt6::Core
        Qt6::Sql
        Metrics
        ThreadPool
        RedisCache
        Entities
        nlohmann_json::nlohmann_json
)

set_target_properties(Persistence PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

endif()
