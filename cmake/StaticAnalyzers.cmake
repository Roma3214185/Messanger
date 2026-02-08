function(enable_cppcheck TARGET)
    if (NOT ENABLE_CPPCHECK)
        return()
    endif()

    find_program(CPPCHECK_EXE cppcheck)
    if (NOT CPPCHECK_EXE)
        message(FATAL_ERROR "cppcheck not found")
    endif()

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

    set_target_properties(${TARGET} PROPERTIES
        CXX_CPPCHECK "${CPPCHECK_EXE};${CPPCHECK_OPTIONS}"
    )
endfunction()

function(enable_clang_tidy TARGET)
    if (NOT ENABLE_CLANG_TIDY)
        return()
    endif()

    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if (NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR "clang-tidy requested but not found")
    endif()

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

function(enable_iwyu TARGET)
endfunction()

if(ENABLE_IWYU)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  else()
    message(SEND_ERROR "include-what-you-use requested but executable not found")
  endif()
endif()


