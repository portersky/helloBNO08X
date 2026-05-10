find_package(Unity REQUIRED)
find_package(CMock REQUIRED)

set(MOCK_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/mocks")
file(MAKE_DIRECTORY "${MOCK_GEN_DIR}")

# Generate a CMock mock from a header and attach it to a target.
# Usage: cmock_generate_mock(<target> <absolute-path-to-header>)
function(cmock_generate_mock target header)
    if (NOT RUBY_EXECUTABLE)
        message(FATAL_ERROR "Ruby is required for CMock generation")
    endif()
    get_filename_component(name "${header}" NAME_WE)
    get_filename_component(header_dir "${header}" DIRECTORY)
    set(mock_src "${MOCK_GEN_DIR}/Mock${name}.c")
    set(mock_hdr "${MOCK_GEN_DIR}/Mock${name}.h")
    add_custom_command(
        OUTPUT  "${mock_src}" "${mock_hdr}"
        COMMAND "${RUBY_EXECUTABLE}" "${CMOCK_SCRIPT}"
                "-o${CMAKE_CURRENT_SOURCE_DIR}/cmock_config.yml"
                "--mock_path=${MOCK_GEN_DIR}"
                "${header}"
        DEPENDS "${header}" "${CMAKE_CURRENT_SOURCE_DIR}/cmock_config.yml"
        COMMENT "CMock: generating Mock${name}"
        VERBATIM
    )
    target_sources("${target}" PRIVATE "${mock_src}")
    target_include_directories("${target}" PRIVATE "${MOCK_GEN_DIR}" "${header_dir}")
endfunction()

set(TEST_TARGETS "")

# bno08x tests — mocks i2c_bus.h so I2C calls are intercepted
add_executable(test_bno08x test_bno08x.c)
target_include_directories(test_bno08x PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(test_bno08x PRIVATE bno08x Unity::Unity CMock::CMock)
target_compile_features(test_bno08x PRIVATE c_std_23)
target_compile_options(test_bno08x PRIVATE ${BASE_OPTIONS})
cmock_generate_mock(test_bno08x "${CMAKE_SOURCE_DIR}/src/i2c_bus.h")
add_test(NAME test_bno08x COMMAND test_bno08x)
list(APPEND TEST_TARGETS test_bno08x)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
            --test-dir "${CMAKE_BINARY_DIR}"
            -C $<CONFIG>
            --output-on-failure
            --progress
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    USES_TERMINAL
    DEPENDS ${TEST_TARGETS}
)

if (ENABLE_COVERAGE)
    set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_DIR}"
        COMMAND ${GCOVR_EXE}
                --gcov-executable "${GCOV_EXECUTABLE}"
                --root "${CMAKE_SOURCE_DIR}"
                --filter "${CMAKE_SOURCE_DIR}/src/"
                --exclude "${CMAKE_SOURCE_DIR}/tests/"
                --exclude ".*Mock.*"
                --exclude ".*unity.*"
                --exclude ".*cmock.*"
                --html-details "${COVERAGE_DIR}/index.html"
                --txt
                --print-summary
                "${CMAKE_BINARY_DIR}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        USES_TERMINAL
        DEPENDS ${TEST_TARGETS}
        COMMENT "Coverage report: ${COVERAGE_DIR}/index.html"
    )
endif()
