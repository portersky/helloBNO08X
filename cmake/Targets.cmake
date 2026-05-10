# add_exec(<name> [LIBS <lib>...] <source_files>...)
#
# Creates an xecutable target with standard project settings applied.
#
# Parameters:
#   - name          Name of the target and resulting executable
#   - LIBS          Optional list of additional libraries to link
#   - source_files  One or more source files to compile
function(add_exec NAME)
    cmake_parse_arguments(ARG "" "" "LIBS;INC" ${ARGN})
    set(SOURCES ${ARG_UNPARSED_ARGUMENTS})
    add_executable(${NAME} ${SOURCES})
    target_include_directories(${NAME} PRIVATE "${PROJECT_SOURCE_DIR}" ${ARG_INC})
    target_compile_features(${NAME} PRIVATE c_std_23 cxx_std_23)
    # target_compile_definitions(${NAME} PRIVATE ${PLATFORM_DEFINTIONS})
    target_compile_options(${NAME} PRIVATE
        -Wall
        -Wextra
        -Wconversion
        -Wpedantic
        -Wshadow
        -Werror
        # C++-only flags applied only when compiling C++ translation units
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions -fno-threadsafe-statics>
    )
    target_link_libraries(${NAME} PRIVATE ${ARG_LIBS})
    source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${SOURCES})
endfunction()

# add_static_lib(<name> [LIBS <lib>...] [source_files...])
#
# Creates a static library target with standard project settings applied.
#
# Parameters:
#   - name          Name of the target and resulting library
#   - LIBS          Optional list of additional libraries to link
#   - source_files  One or more source files to compile
function(add_static_lib NAME)
    cmake_parse_arguments(ARG "" "" "LIBS" ${ARGN})
    set(SOURCES ${ARG_UNPARSED_ARGUMENTS})
    add_library(${NAME} STATIC ${SOURCES})
    target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}")
    target_compile_features(${NAME} PUBLIC cxx_std_23)
    target_compile_features(${NAME} PRIVATE c_std_23)
    target_link_libraries(${NAME}
        PUBLIC
        ${ARG_LIBS}
    )
    target_compile_definitions(${NAME} PRIVATE ${PLATFORM_DEFINTIONS})
    target_compile_options(${NAME} PRIVATE ${BASE_OPTIONS})
    source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${SOURCES})
endfunction()
