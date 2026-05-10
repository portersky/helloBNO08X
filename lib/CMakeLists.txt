# lib/CMakeLists.txt
# Defines four targets consumed by the firmware executable:
#   cmsis   — header-only INTERFACE library
#   startup — static lib (startup asm + system init), linked whole
#   hal     — static lib (all HAL/LL drivers)
#   bsp     — static lib (Nucleo BSP)
#
# All hardware-specific paths come from cmake/MCU.cmake via TARGET_MCU.

# MCU.cmake is already included by the toolchain file, but include() is
# idempotent so this is safe and makes the dependency explicit here too.
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/MCU.cmake")

# ------------------------------------------------------------------------------
# cmsis — header only
# ------------------------------------------------------------------------------

add_library(cmsis INTERFACE)
target_include_directories(cmsis INTERFACE
    "${MCU_CMSIS_DEVICE_INC}"
    "${CMAKE_CURRENT_LIST_DIR}/CMSIS/Include"
)

# ------------------------------------------------------------------------------
# startup
# ------------------------------------------------------------------------------

# Prefix startup sources with the lib/ directory
list(TRANSFORM MCU_STARTUP_SRC PREPEND "${CMAKE_CURRENT_LIST_DIR}/")

add_library(startup STATIC ${MCU_STARTUP_SRC})
target_include_directories(startup PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/startup"
)
target_link_libraries(startup PUBLIC cmsis)

# ------------------------------------------------------------------------------
# hal — all HAL + LL driver sources for the selected MCU
# ------------------------------------------------------------------------------

file(GLOB _HAL_SOURCES "${MCU_HAL_SRC_DIR}/*.c")
list(FILTER _HAL_SOURCES EXCLUDE REGEX ".*_template\\.c$")

add_library(hal STATIC ${_HAL_SOURCES})
target_include_directories(hal PUBLIC "${MCU_HAL_INC}")
target_compile_options(hal PRIVATE -Wno-unused-parameter -Wno-pedantic -Wno-conversion)
target_link_libraries(hal PUBLIC cmsis startup)

# ------------------------------------------------------------------------------
# bsp — Nucleo board support
# ------------------------------------------------------------------------------

# Prefix BSP sources with the lib/ directory
list(TRANSFORM MCU_BSP_SRC PREPEND "${CMAKE_CURRENT_LIST_DIR}/")

add_library(bsp STATIC ${MCU_BSP_SRC})
target_include_directories(bsp PUBLIC "${MCU_BSP_INC}")
target_compile_options(bsp PRIVATE -Wno-unused-parameter -Wno-pedantic -Wno-conversion)
target_link_libraries(bsp PUBLIC hal)
