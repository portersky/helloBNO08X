# cmake/arm-none-eabi-toolchain.cmake
#
# CMake toolchain file for ARM Cortex-M bare-metal targets.
# Automatically downloads the ARM GNU toolchain into <project_root>/.tools/
# if it is not already present.
#
# Usage:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake

set(TOOLCHAIN_VERSION "15.2.rel1")
set(TOOLCHAIN_PREFIX  "arm-none-eabi")

# Resolve .tools/ relative to this file's location (cmake/../.tools)
set(TOOLCHAIN_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.tools")
cmake_path(NORMAL_PATH TOOLCHAIN_ROOT)

# Platform-specific archive name and extracted directory
if(CMAKE_HOST_WIN32)
    set(_ARCH_STEM "arm-gnu-toolchain-${TOOLCHAIN_VERSION}-mingw-w64-x86_64-${TOOLCHAIN_PREFIX}")
    set(_ARCHIVE   "${_ARCH_STEM}.zip")
else()
    set(_ARCH_STEM "arm-gnu-toolchain-${TOOLCHAIN_VERSION}-x86_64-${TOOLCHAIN_PREFIX}")
    set(_ARCHIVE   "${_ARCH_STEM}.tar.xz")
endif()

set(_URL          "https://developer.arm.com/-/media/Files/downloads/gnu/${TOOLCHAIN_VERSION}/binrel/${_ARCHIVE}")
set(_SUBDIR_ROOT  "${TOOLCHAIN_ROOT}/${_ARCH_STEM}")   # layout after a fresh download
set(_GCC_EXE      "${TOOLCHAIN_PREFIX}-gcc${CMAKE_HOST_EXECUTABLE_SUFFIX}")

# Prefer a flat install (.tools/bin/) that the user may have set up manually,
# then fall back to the subdirectory layout that a fresh download produces.
if(EXISTS "${TOOLCHAIN_ROOT}/bin/${_GCC_EXE}")
    set(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_ROOT}/bin")
    set(_SYSROOT          "${TOOLCHAIN_ROOT}")
elseif(EXISTS "${_SUBDIR_ROOT}/bin/${_GCC_EXE}")
    set(TOOLCHAIN_BIN_DIR "${_SUBDIR_ROOT}/bin")
    set(_SYSROOT          "${_SUBDIR_ROOT}")
else()
    # Neither layout found — download and extract
    message(STATUS "ARM GNU toolchain not found — downloading...")
    file(MAKE_DIRECTORY "${TOOLCHAIN_ROOT}")

    set(_ARCHIVE_PATH "${TOOLCHAIN_ROOT}/${_ARCHIVE}")
    file(DOWNLOAD "${_URL}" "${_ARCHIVE_PATH}"
        SHOW_PROGRESS
        STATUS _STATUS)
    list(GET _STATUS 0 _STATUS_CODE)
    if(NOT _STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Toolchain download failed: ${_STATUS}")
    endif()

    message(STATUS "Extracting ARM GNU toolchain...")
    file(ARCHIVE_EXTRACT INPUT "${_ARCHIVE_PATH}" DESTINATION "${TOOLCHAIN_ROOT}")
    message(STATUS "ARM GNU toolchain ready: ${_SUBDIR_ROOT}/bin")

    set(TOOLCHAIN_BIN_DIR "${_SUBDIR_ROOT}/bin")
    set(_SYSROOT          "${_SUBDIR_ROOT}")
endif()

# --- Cross-compilation settings ---

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Prevent CMake from link-testing the compiler (no OS / no libc on bare-metal)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER   "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}-gcc${CMAKE_HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}-g++${CMAKE_HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}-gcc${CMAKE_HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}-objcopy${CMAKE_HOST_EXECUTABLE_SUFFIX}")
set(CMAKE_SIZE         "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}-size${CMAKE_HOST_EXECUTABLE_SUFFIX}")

# Only search for headers/libs inside the sysroot, not the host system
set(CMAKE_FIND_ROOT_PATH "${_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# --- MCU flags (shared by all targets in this project) ---
# MCU_CPU_FLAGS and MCU_DEFINE are set by cmake/MCU.cmake based on TARGET_MCU.

include("${CMAKE_CURRENT_LIST_DIR}/MCU.cmake")

add_compile_options(
    ${MCU_CPU_FLAGS}
    -D${MCU_DEFINE}
    -fdata-sections
    -ffreestanding
    -ffunction-sections
    -nostdlib
    -Wall
    -Wextra
    -Wconversion
    -Wpedantic
    -Wshadow
)

add_link_options(
    ${MCU_CPU_FLAGS}
    --specs=nano.specs
    -Wl,--gc-sections
)
