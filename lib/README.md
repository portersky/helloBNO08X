# Library Directory Structure

This document describes the layout and contents of the `lib` directory, which
contains low-level software components for STM32 microcontroller projects.

## Table of Contents

  - [CMSIS](#cmsis)
    - [CMSIS Version 5](#cmsis-version-5)
    - [Device Directory](#device-directory)
  - [link](#link)
  - [startup](#startup)
  - [stm32xx](#stm32xx)
    - [Core](#core)
    - [Drivers](#drivers)
    - [Middlewares](#middlewares)

## CMSIS

CMSIS (ARM® Cortex® Microcontroller Software Interface Standard) provides a
common hardware abstraction layer for the Cortex-M processor series.

### CMSIS Version 5

The CMSIS\_5 repository contains:

  - Core CMSIS files (core definitions, startup code, access layer)
  - DSP, RTOS, and NN libraries
  - Packs and tools for device support

Reference: [https://github.com/ARM-software/CMSIS\_5](https://github.com/ARM-software/CMSIS_5)

### Device Directory

Under `CMSIS/Device` you will find device-specific support packages organized by
vendor and series. For example:

```
CMSIS/
└── Device/
    └── ST/
        └── STM32U5xx/
            ├── Include/
            │   ├── stm32u5xx.h
            │   └── system_stm32u5xx.h
            ├── Source/
            │   └── system_stm32u5xx.c
            └── ...
```

These folders include header files and system initialization code for each STM32
microcontroller series.

## link

Linker script files (`.ld`) that define the memory layout, section placement, and
startup vectors for STM32 microcontrollers.

## startup

Startup files, typically in assembly (.s) or C, responsible for initial CPU and
RAM setup, vector table, and system initialization for:

  - STM32 core
  - HAL framework
  - Nucleo board support
  - System clock configuration

## stm32xx

The `stm32xx` directory contains STMicroelectronics-specific libraries and
components, where `xx` is the series identifier corresponding to the MCU family
(e.g., U5, H5). ST refers to these as "series", denoting tiers of performance
and feature sets.

Example structure:

```
stm32xx/
├── Drivers/
│   ├── STM32U5xx_HAL_Driver/
│   ├── STM32U5xx_LL_Driver/
│   └── BSP/
│       └── STM32xxxx_Nucleo/
└── Middlewares/
    ├── USB_Device/
    ├── FatFS/
    └── LwIP/
```

The components are organized into:

### Drivers

  - HAL (Hardware Abstraction Layer) and LL (Low Layer) drivers for peripherals
    (GPIO, USART, ADC, etc.), providing high-level and performance-optimized
    hardware access.

  - BSP (Board Support Package) directory containing board-specific
    configurations, drivers, and examples tailored for ST evaluation boards or
    custom hardware setups.

### Middlewares

Use for additional software stacks such as USB, File System (FatFS), TCP/IP (LwIP), and
more.

