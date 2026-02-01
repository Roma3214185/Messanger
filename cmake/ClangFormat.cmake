function(enable_clang_format TARGET)
    if(NOT ENABLE_CLANG_FORMAT)
        return()
    endif()

    find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-15 PATHS /opt/homebrew/bin)

    if(NOT CLANG_FORMAT_EXE)
        message(FATAL_ERROR "clang-format requested but not found")
    endif()

    # get_target_property(TARGET_SOURCES ${TARGET} SOURCES)
    # if(NOT TARGET_SOURCES)
    #     message(WARNING "Target ${TARGET} has no sources for clang-format")
    #     return()
    # endif()

    set(FORMAT_FILES)

    file(GLOB_RECURSE TARGET_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    )
    list(APPEND FORMAT_FILES ${TARGET_FILES})

    if(FORMAT_FILES)
        add_custom_target(
            clang-format-${TARGET}
            COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-format on ${TARGET} sources..."
            VERBATIM
        )
        add_dependencies(${TARGET} clang-format-${TARGET})
    else()
        message(WARNING "No source/header files found for clang-format in target ${TARGET}")
    endif()
endfunction()
