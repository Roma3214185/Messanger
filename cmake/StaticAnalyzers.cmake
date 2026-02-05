# -----------------------------------------------------------------------------
# Enable Cppcheck for a target
# -----------------------------------------------------------------------------
function(enable_cppcheck TARGET)
    if (NOT ENABLE_CPPCHECK)
        return()
    endif()

    find_program(CPPCHECK_EXE cppcheck)
    if (NOT CPPCHECK_EXE)
        message(FATAL_ERROR "cppcheck not found")
    endif()

    message(STATUS "BINARY_DIR=${CMAKE_BINARY_DIR}")

    set(CPPCHECK_OPTIONS
        --max-ctu-depth=3
        --enable=warning,performance,portability
        --inline-suppr
        --suppress=missingIncludeSystem
        -i${CMAKE_SOURCE_DIR}/external
        -i${CMAKE_BINARY_DIR}/
        --suppress=*:*/_deps/*
        --suppress=*:*/external/*
        --suppress=unmatchedSuppression
        --suppress=returnByReference
        --language=c++
        --library=qt
        --error-exitcode=666
    )

    #cppcheck --max-ctu-depth=3 --enable=all --inline-suppr --suppress=*:*thrust/complex* --suppress=missingInclude --suppress=syntaxError --suppress=unmatchedSuppression --suppress=preprocessorErrorDirective --language=c++ --std=c++14 --error-exitcode=666


    message(STATUS "cppcheck runs on target: ${TARGET}")

    set_target_properties(${TARGET} PROPERTIES
        CXX_CPPCHECK "${CPPCHECK_EXE};${CPPCHECK_OPTIONS}"
    )
endfunction()

# -----------------------------------------------------------------------------
# Disable Cppcheck for a target
# -----------------------------------------------------------------------------
function(disable_cppcheck target)
  set_target_properties(${target} PROPERTIES
    C_CPPCHECK ""
    CXX_CPPCHECK ""
  )
endfunction()

# -----------------------------------------------------------------------------
# Disable Cppcheck for a directory
# -----------------------------------------------------------------------------
function(disable_cppcheck_in_dir dir)
  get_property(targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
  foreach(t ${targets})
    set_target_properties(${t} PROPERTIES
      C_CPPCHECK ""
      CXX_CPPCHECK ""
    )
  endforeach()
endfunction()

# -----------------------------------------------------------------------------
# Enable Clang-Tidy for a target
# -----------------------------------------------------------------------------
function(enable_clang_tidy TARGET)
    if (NOT ENABLE_CLANG_TIDY)
        return()
    endif()

    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if (NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR "clang-tidy requested but not found")
    endif()

    # Only analyze project directories (common, Backend, Frontend), exclude external deps
    set(HEADER_FILTER_REGEX "^${CMAKE_SOURCE_DIR}/(common|Backend|Frontend)/.*")

    set(CLANG_TIDY_OPTIONS
        -extra-arg=-Wno-unknown-warning-option
        -header-filter=${HEADER_FILTER_REGEX}
    )

    message(STATUS "clang-tidy runs on target: ${TARGET}")

    set_target_properties(${TARGET} PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXE};${CLANG_TIDY_OPTIONS}"
    )
endfunction()

# -----------------------------------------------------------------------------
# Enable Include-What-You-Use (IWYU) for a target
# -----------------------------------------------------------------------------
function(enable_iwyu TARGET)
    if (NOT ENABLE_IWYU)
        return()
    endif()

    find_program(IWYU_EXE include-what-you-use)
    if (NOT IWYU_EXE)
        message(FATAL_ERROR "include-what-you-use not found")
    endif()

    set(IWYU_OPTIONS
        #-Xiwyu --mapping_file=${CMAKE_SOURCE_DIR}/.iwyu.imp
        -Xiwyu --exclude=${CMAKE_SOURCE_DIR}/external
        #-Xiwyu --exclude=${CMAKE_SOURCE_DIR}/_deps
        -Xiwyu --exclude=${CMAKE_BINARY_DIR}/_deps
        -Xiwyu --verbose=3
    )

    message(STATUS "IWYU runs on target: ${TARGET}")
    message(STATUS "IWYU sources for ${TARGET}: ${SRCS}")

    set_target_properties(${TARGET} PROPERTIES
        CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXE};${IWYU_OPTIONS}"
    )
endfunction()
