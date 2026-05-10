# cmake/MCU.cmake
#
# Maps TARGET_MCU to all hardware-specific build parameters.
# Include this file early; it sets the following variables:
#
#   MCU_CPU_FLAGS       — compiler/linker -mcpu/-mfpu/-mfloat-abi/-mthumb flags
#   MCU_DEFINE          — preprocessor define identifying the device (e.g. STM32U545xx)
#   MCU_CMSIS_DEVICE_INC— path (relative to lib/) to the CMSIS device header directory
#   MCU_HAL_INC         — path (relative to lib/) to the HAL Inc/ directory
#   MCU_HAL_SRC_DIR     — path (relative to lib/) to the HAL Src/ directory
#   MCU_BSP_INC         — path (relative to lib/) to the BSP Inc/ directory
#   MCU_BSP_SRC         — list of BSP source files (relative to lib/)
#   MCU_STARTUP_SRC     — list of startup source files (relative to lib/startup/)
#   MCU_LINKER_SCRIPT   — absolute path to the flash linker script

set(TARGET_MCU "STM32U545xx" CACHE STRING
    "Target MCU identifier. Supported: STM32U545xx, STM32H503xx")
message(STATUS "Target MCU: ${TARGET_MCU}")

# Helper used below — resolves a lib/-relative path to absolute
macro(_lib_path VAR REL)
    set(${VAR} "${CMAKE_CURRENT_LIST_DIR}/../lib/${REL}")
    cmake_path(NORMAL_PATH ${VAR})
endmacro()

# ------------------------------------------------------------------------------
if(TARGET_MCU STREQUAL "STM32U545xx")
# ------------------------------------------------------------------------------

    set(MCU_CPU_FLAGS
        -mcpu=cortex-m33
        -mfloat-abi=hard
        -mfpu=fpv5-sp-d16
        -mthumb
    )
    set(MCU_DEFINE STM32U545xx)
    _lib_path(MCU_CMSIS_DEVICE_INC "CMSIS/Device/ST/STM32U5xx/Include")
    _lib_path(MCU_HAL_INC          "stm32u5/Drivers/STM32U5xx_HAL_Driver/Inc")
    _lib_path(MCU_HAL_SRC_DIR      "stm32u5/Drivers/STM32U5xx_HAL_Driver/Src")
    _lib_path(MCU_BSP_INC          "stm32u5/Drivers/BSP/STM32U5xx_Nucleo")
    set(MCU_BSP_SRC
        stm32u5/Drivers/BSP/STM32U5xx_Nucleo/stm32u5xx_nucleo.c
    )
    set(MCU_STARTUP_SRC
        startup/startup_stm32u545retx.s
        startup/system_stm32u5xx.c
    )
    _lib_path(MCU_LINKER_SCRIPT "link/STM32U545RETXQ_FLASH.ld")
    set(MCU_OPENOCD_TARGET "target/stm32u5x.cfg")

# ------------------------------------------------------------------------------
elseif(TARGET_MCU STREQUAL "STM32H503xx")
# ------------------------------------------------------------------------------

    set(MCU_CPU_FLAGS
        -mcpu=cortex-m33
        -mfloat-abi=hard
        -mfpu=fpv5-sp-d16
        -mthumb
    )
    set(MCU_DEFINE STM32H503xx)
    _lib_path(MCU_CMSIS_DEVICE_INC "CMSIS/Device/ST/STM32H5xx/Include")
    _lib_path(MCU_HAL_INC          "stm32h5/Drivers/STM32H5xx_HAL_Driver/Inc")
    _lib_path(MCU_HAL_SRC_DIR      "stm32h5/Drivers/STM32H5xx_HAL_Driver/Src")
    _lib_path(MCU_BSP_INC          "stm32h5/Drivers/BSP/STM32H5xx_Nucleo")
    set(MCU_BSP_SRC
        stm32h5/Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.c
    )
    set(MCU_STARTUP_SRC
        startup/startup_stm32h503xx.s
        startup/system_stm32h5xx.c
    )
    _lib_path(MCU_LINKER_SCRIPT "link/STM32H503CBTX_FLASH.ld")
    set(MCU_OPENOCD_TARGET "target/stm32h5x.cfg")

# ------------------------------------------------------------------------------
else()
# ------------------------------------------------------------------------------

    message(FATAL_ERROR
        "Unsupported TARGET_MCU '${TARGET_MCU}'. "
        "Add an entry for it in cmake/MCU.cmake."
    )

endif()
