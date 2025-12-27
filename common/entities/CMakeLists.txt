cmake_minimum_required(VERSION 3.22)

if (NOT TARGET Entities)

    add_library(Entities INTERFACE)
    #target_compile_features(Entities PUBLIC cxx_std_20)

    target_include_directories(Entities INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(Entities INTERFACE
        nlohmann_json::nlohmann_json
        Crow::Crow
        Metrics
    )

    enable_clang_tidy(Entities)
    enable_cppcheck(Entities)
    enable_iwyu(Entities)

endif()

