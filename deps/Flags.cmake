# ==============================================================================
# Compiler Flags
# ==============================================================================
# Sets BASE_OPTIONS (warning flags) and BASE_DEFINITIONS.
# Apply per-target via target_compile_options / target_compile_definitions
# to avoid polluting fetched dependencies.
#
# Requires: Platform.cmake (for IS_CLANG_OR_GCC / IS_MSVC)
# ==============================================================================

set(BASE_DEFINITIONS "")
set(BASE_LIBRARIES   "")

set(BASE_OPTIONS "")
if (IS_CLANG_OR_GCC)
    set(BASE_OPTIONS
        "-Wall"
        "-Wextra"
        "-Werror"
    )
elseif (IS_MSVC)
    set(BASE_OPTIONS
        "/W4"
        "/WX"
        "/utf-8"
    )
endif()
