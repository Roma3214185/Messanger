cmake_minimum_required(VERSION 3.22)

if (NOT TARGET Entities)
    find_package(nlohmann_json REQUIRED)

    add_library(Entities STATIC
        src/utils.cpp
    )

    target_include_directories(Entities PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(Entities PUBLIC
        nlohmann_json::nlohmann_json
        Crow::Crow
        ProjectOptions
        Metrics
    )

    enable_clang_tidy(Entities)
    enable_cppcheck(Entities)
    enable_iwyu(Entities)
    enable_sanitizers(Entities)
    enable_clang_format(Entities)

endif()

