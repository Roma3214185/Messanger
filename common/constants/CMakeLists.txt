cmake_minimum_required(VERSION 3.22)

if (NOT TARGET Constants)
    add_library(Constants INTERFACE)

    target_include_directories(Constants INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    enable_clang_tidy(Constants)
    enable_cppcheck(Constants)
    enable_iwyu(Constants)

endif()
