# ==============================================================================
# Find CMock
# ==============================================================================
# This module fetches the CMock mocking framework (depends on Unity).
#
# Targets provided:
#   CMock::CMock - The CMock library target
#
# Variables set:
#   CMock_FOUND       - TRUE if CMock is available
#   CMock_LIBRARIES   - The CMock library target (CMock::CMock)
#   CMock_INCLUDE_DIR - Include directories for CMock
#   CMock_VERSION     - Version of CMock (if available)
#
# Generator variables (set when Ruby is found):
#   CMOCK_SCRIPT      - Path to lib/cmock.rb
#   RUBY_EXECUTABLE   - Path to ruby interpreter
# ==============================================================================

if (DEFINED _FINDCMOCK_INCLUDED)
    return()
endif()
set(_FINDCMOCK_INCLUDED TRUE)

find_package(Unity REQUIRED)

if (DEFINED CMock_FIND_VERSION AND NOT CMock_FIND_VERSION STREQUAL "")
    set(CMOCK_VERSION "${CMock_FIND_VERSION}")
else()
    set(CMOCK_VERSION "2.6.0")
endif()

message(STATUS "Fetching CMock ${CMOCK_VERSION}")

include(FetchContent)

FetchContent_Declare(
    cmock
    URL https://github.com/ThrowTheSwitch/CMock/archive/refs/tags/v${CMOCK_VERSION}.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# CMock uses Meson — bypass its build system and compile src/cmock.c directly.
# FetchContent_MakeAvailable cannot be used here (no CMakeLists.txt in CMock),
# so we call FetchContent_Populate directly and opt into the old policy.
cmake_policy(PUSH)
cmake_policy(SET CMP0169 OLD)
FetchContent_GetProperties(cmock)
if (NOT cmock_POPULATED)
    FetchContent_Populate(cmock)
endif()
cmake_policy(POP)

# The Ruby generator expects vendor/unity/auto/type_sanitizer.rb — populate
# it from the Unity source we already have rather than needing a git submodule
set(_cmock_vendor_auto "${cmock_SOURCE_DIR}/vendor/unity/auto")
if (NOT EXISTS "${_cmock_vendor_auto}/type_sanitizer.rb")
    file(MAKE_DIRECTORY "${_cmock_vendor_auto}")
    file(COPY "${unity_SOURCE_DIR}/auto/" DESTINATION "${_cmock_vendor_auto}")
endif()

if (NOT TARGET cmock)
    add_library(cmock STATIC "${cmock_SOURCE_DIR}/src/cmock.c")
    target_include_directories(cmock PUBLIC
        "${cmock_SOURCE_DIR}/src"
        "${unity_SOURCE_DIR}/src"
    )
    target_link_libraries(cmock PUBLIC unity)
endif()

if (NOT TARGET CMock::CMock)
    add_library(CMock::CMock ALIAS cmock)
endif()

set(CMock_FOUND        TRUE)
set(CMock_LIBRARIES    CMock::CMock)
set(CMock_VERSION      "${CMOCK_VERSION}")
set(CMock_INCLUDE_DIR  "${cmock_SOURCE_DIR}/src")
set(CMOCK_SCRIPT       "${cmock_SOURCE_DIR}/lib/cmock.rb" CACHE FILEPATH "Path to CMock Ruby generator")

if (CMock_INCLUDE_DIR AND TARGET cmock)
    set_target_properties(cmock PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMock_INCLUDE_DIR}"
    )
endif()

find_program(RUBY_EXECUTABLE ruby)
if (NOT RUBY_EXECUTABLE)
    message(WARNING "Ruby not found — CMock code generation unavailable")
endif()

set(CMOCK_LICENSE_FILE "${cmock_SOURCE_DIR}/LICENSE.txt" CACHE FILEPATH "Path to CMock license file")
