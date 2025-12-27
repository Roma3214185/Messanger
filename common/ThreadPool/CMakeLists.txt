cmake_minimum_required(VERSION 3.22)

if (NOT TARGET ThreadPool)

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    add_library(ThreadPool STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/threadpool.cpp
    )

    target_compile_features(ThreadPool PUBLIC cxx_std_20)

    enable_clang_tidy(ThreadPool)
    enable_cppcheck(ThreadPool)
    enable_iwyu(ThreadPool)

    target_include_directories(ThreadPool PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

endif()
