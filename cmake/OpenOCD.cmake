# cmake/OpenOCD.cmake
#
# Downloads xpack OpenOCD and adds a 'flash' target that programs the board.
# The download URL is chosen automatically based on the host platform; set
# OPENOCD_URL in the cache to override.
#
# Cache variables:
#   OPENOCD_VERSION   — xpack OpenOCD version to download (default: 0.12.0-7)
#                       Releases: https://github.com/xpack-dev-tools/openocd-xpack/releases
#   OPENOCD_URL       — Full URL override; auto-derived from OPENOCD_VERSION if unset
#   OPENOCD_INTERFACE — OpenOCD interface config (relative to scripts dir)
#
# Usage:
#   cmake --build build --target flash
#   cmake --build build --target debug   # starts GDB server on :3333

include(FetchContent)

# ---------- platform detection ------------------------------------------------
set(OPENOCD_VERSION "0.12.0-7" CACHE STRING "xpack OpenOCD version to download")

if(NOT DEFINED CACHE{OPENOCD_URL})
    # Use host variables — CMAKE_SYSTEM_NAME is "Generic" for bare-metal targets.
    set(_ocd_base "https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v${OPENOCD_VERSION}/xpack-openocd-${OPENOCD_VERSION}")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(_ocd_url "${_ocd_base}-win32-x64.zip")
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        set(_ocd_url "${_ocd_base}-linux-x64.tar.gz")
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(_ocd_url "${_ocd_base}-darwin-arm64.tar.gz")
        else()
            set(_ocd_url "${_ocd_base}-darwin-x64.tar.gz")
        endif()
    else()
        message(FATAL_ERROR
            "No default OPENOCD_URL for host '${CMAKE_HOST_SYSTEM_NAME}'. "
            "Set -DOPENOCD_URL=<url> on the cmake command line.")
    endif()
    set(OPENOCD_URL "${_ocd_url}" CACHE STRING "URL to xpack OpenOCD release archive")
endif()

set(OPENOCD_INTERFACE "interface/stlink.cfg"
    CACHE STRING "OpenOCD interface config file (relative to its scripts dir)")

# ---------- download ----------------------------------------------------------
FetchContent_Declare(openocd_dist
    URL "${OPENOCD_URL}"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(openocd_dist)

# ---------- locate executable -------------------------------------------------
# Search both stripped (no top-level dir) and non-stripped archive layouts.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(_ocd_exe_name "openocd.exe")
else()
    set(_ocd_exe_name "openocd")
endif()

file(GLOB _ocd_exe
    "${openocd_dist_SOURCE_DIR}/bin/${_ocd_exe_name}"
    "${openocd_dist_SOURCE_DIR}/*/bin/${_ocd_exe_name}"
)
if(NOT _ocd_exe)
    message(FATAL_ERROR
        "${_ocd_exe_name} not found under ${openocd_dist_SOURCE_DIR}. "
        "Check OPENOCD_URL or inspect the extracted directory.")
endif()
list(GET _ocd_exe 0 _OPENOCD_EXE)

# Scripts live at <package-root>/openocd/share/openocd/scripts.
cmake_path(GET _OPENOCD_EXE PARENT_PATH _ocd_bin_dir)  # …/bin
cmake_path(GET _ocd_bin_dir PARENT_PATH _ocd_root)      # …/<package-root>
set(_OPENOCD_SCRIPTS "${_ocd_root}/openocd/share/openocd/scripts")

message(STATUS "OpenOCD exe:     ${_OPENOCD_EXE}")
message(STATUS "OpenOCD scripts: ${_OPENOCD_SCRIPTS}")

# ---------- debug target ------------------------------------------------------
# Starts OpenOCD as a GDB server on port 3333 and stays running.
# Connect with: arm-none-eabi-gdb -ex "target remote :3333" build/firmware.elf
add_custom_target(debug
    COMMAND "${_OPENOCD_EXE}"
        -s "${_OPENOCD_SCRIPTS}"
        -f "${OPENOCD_INTERFACE}"
        -f "${MCU_OPENOCD_TARGET}"
    COMMENT "OpenOCD GDB server on :3333 — connect with arm-none-eabi-gdb (Ctrl+C to stop)"
    USES_TERMINAL
    VERBATIM
)

# ---------- flash target ------------------------------------------------------
add_custom_target(flash
    COMMAND "${_OPENOCD_EXE}"
        -s "${_OPENOCD_SCRIPTS}"
        -f "${OPENOCD_INTERFACE}"
        -f "${MCU_OPENOCD_TARGET}"
        -c "program $<TARGET_FILE:fw> verify reset exit"
    DEPENDS fw
    COMMENT "Flashing ${TARGET_MCU} via OpenOCD (${OPENOCD_INTERFACE})"
    USES_TERMINAL
    VERBATIM
)
