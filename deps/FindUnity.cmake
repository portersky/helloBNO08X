# ==============================================================================
# Find Unity
# ==============================================================================
# This module fetches the Unity unit testing framework.
#
# Targets provided:
#   Unity::Unity - The Unity library target
#
# Variables set:
#   Unity_FOUND       - TRUE if Unity is available
#   Unity_LIBRARIES   - The Unity library target (Unity::Unity)
#   Unity_INCLUDE_DIR - Include directories for Unity
#   Unity_VERSION     - Version of Unity (if available)
# ==============================================================================

if (DEFINED _FINDUNITY_INCLUDED)
    return()
endif()
set(_FINDUNITY_INCLUDED TRUE)

if (DEFINED Unity_FIND_VERSION AND NOT Unity_FIND_VERSION STREQUAL "")
    set(UNITY_VERSION "${Unity_FIND_VERSION}")
else()
    set(UNITY_VERSION "2.6.1")
endif()

message(STATUS "Fetching Unity ${UNITY_VERSION}")

include(FetchContent)

FetchContent_Declare(
    unity
    URL https://github.com/ThrowTheSwitch/Unity/archive/refs/tags/v${UNITY_VERSION}.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(unity)

# Unity sets INTERFACE_SYSTEM_INCLUDE_DIRECTORIES to a path inside the build
# tree, which CMake rejects on newer versions. The path stays in
# INTERFACE_INCLUDE_DIRECTORIES so headers are still found.
if (TARGET unity)
    set_target_properties(unity PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "")
endif()

if (NOT TARGET Unity::Unity)
    if (TARGET unity)
        add_library(Unity::Unity ALIAS unity)
    else()
        message(FATAL_ERROR "Could not fetch Unity; no target unity or Unity::Unity available")
    endif()
endif()

set(Unity_FOUND        TRUE)
set(Unity_LIBRARIES    Unity::Unity)
set(Unity_VERSION      "${UNITY_VERSION}")
set(Unity_INCLUDE_DIR  "${unity_SOURCE_DIR}/src")


if (TARGET unity)
    target_compile_definitions(unity PUBLIC
        UNITY_OUTPUT_COLOR
        UNITY_INCLUDE_PRINT_FORMATTED
    )
endif()

set(UNITY_LICENSE_FILE "${unity_SOURCE_DIR}/LICENSE.txt" CACHE FILEPATH "Path to Unity license file")
