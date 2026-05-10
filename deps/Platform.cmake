# ==============================================================================
# Platform Detection
# ==============================================================================
# This module detects the current platform and compiler, setting IS_* variables
# that can be used throughout the build system for conditional logic.
#
# Compiler flags set:
#   IS_CLANG_OR_GCC  - TRUE if using Clang or GCC compiler
#   IS_MSVC          - TRUE if using Microsoft Visual C++ compiler
#
# Platform flags set:
#   IS_WINDOWS       - TRUE if building for Windows
#   IS_LINUX         - TRUE if building for Linux
#   IS_MACOS         - TRUE if building for macOS
#   IS_IOS           - TRUE if building for iOS
#   IS_ANDROID       - TRUE if building for Android
#   IS_EMSCRIPTEN    - TRUE if building for WebAssembly via Emscripten
# ==============================================================================

# ------------------------------------------------------------------------------
# Compiler Detection
# ------------------------------------------------------------------------------
set(IS_CLANG_OR_GCC FALSE)
set(IS_MSVC         FALSE)

if(MSVC)
    set(IS_MSVC TRUE)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    set(IS_CLANG_OR_GCC TRUE)
endif()

# ------------------------------------------------------------------------------
# Platform Detection
# ------------------------------------------------------------------------------
set(IS_WINDOWS    FALSE)
set(IS_LINUX      FALSE)
set(IS_MACOS      FALSE)
set(IS_IOS        FALSE)
set(IS_ANDROID    FALSE)
set(IS_EMSCRIPTEN FALSE)

if(EMSCRIPTEN)
    message(STATUS "Platform: Emscripten")
    set(IS_EMSCRIPTEN TRUE)
elseif(ANDROID)
    message(STATUS "Platform: Android")
    set(IS_ANDROID TRUE)
elseif(APPLE)
    if(IOS)
        message(STATUS "Platform: iOS")
        set(IS_IOS TRUE)
    else()
        message(STATUS "Platform: macOS")
        set(IS_MACOS TRUE)
    endif()
elseif(WIN32)
    message(STATUS "Platform: Windows")
    set(IS_WINDOWS TRUE)
elseif(UNIX)
    message(STATUS "Platform: Linux")
    set(IS_LINUX TRUE)
else()
    message(FATAL_ERROR "Unknown platform!")
endif()
