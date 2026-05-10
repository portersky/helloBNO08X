cmake_minimum_required(VERSION 3.25)
project(helloBNO08X VERSION 0.0.0 LANGUAGES C CXX ASM)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps"
)

if (CMAKE_CROSSCOMPILING)
  # ---- Firmware build (arm-none-eabi) ----

  include(cmake/Targets.cmake)
  include(cmake/MCU.cmake)

  add_compile_options(
        $<$<CONFIG:Debug>:-g3>
        $<$<CONFIG:Debug>:-Og>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-g3>
    )

  add_subdirectory(lib)

  add_library(i2c_bus STATIC src/i2c_bus.c)
  target_include_directories(i2c_bus PUBLIC "${PROJECT_SOURCE_DIR}/src")
  target_link_libraries(i2c_bus PUBLIC hal)

  add_library(bno08x STATIC src/bno08x.c)
  target_include_directories(bno08x PUBLIC "${PROJECT_SOURCE_DIR}/src")
  target_link_libraries(bno08x PUBLIC i2c_bus)

  add_library(uart_log STATIC src/uart_log.c)
  target_include_directories(uart_log PUBLIC "${PROJECT_SOURCE_DIR}/src")
  target_link_libraries(uart_log PUBLIC hal)

  add_exec(fw
        "src/main.c"
        "src/syscalls.c"
        "src/sysmem.c"
        INC ${PROJECT_SOURCE_DIR}/src
        LIBS bno08x uart_log bsp hal cmsis
    )

  target_link_libraries(fw PRIVATE -Wl,--whole-archive startup -Wl,--no-whole-archive)

  set(_LINKER_SCRIPT "${MCU_LINKER_SCRIPT}")
  target_link_options(fw PRIVATE
        -T${_LINKER_SCRIPT}
        -Wl,-Map=${CMAKE_BINARY_DIR}/output.map
    )
  set_target_properties(fw PROPERTIES
        OUTPUT_NAME   firmware
        SUFFIX        .elf
        LINK_DEPENDS  ${_LINKER_SCRIPT}
    )

  add_custom_command(
        TARGET fw POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:fw> ${CMAKE_BINARY_DIR}/firmware.bin
        COMMENT "Generating firmware.bin"
    )

  include(cmake/OpenOCD.cmake)

  cmake_path(GET CMAKE_OBJCOPY PARENT_PATH _toolchain_bin)
  add_custom_target(gdb
        COMMAND "${_toolchain_bin}/arm-none-eabi-gdb${CMAKE_HOST_EXECUTABLE_SUFFIX}"
            -ex "target remote :3333"
            -ex "load"
            -ex "monitor reset init"
            $<TARGET_FILE:fw>
        DEPENDS fw
        COMMENT "arm-none-eabi-gdb — requires 'debug' target running in another terminal"
        USES_TERMINAL
        VERBATIM
    )

else()
  # ---- Host test build ----

  include(Platform)
  include(Flags)
  include(Coverage)

  add_library(bno08x STATIC src/bno08x.c)
  target_include_directories(bno08x PUBLIC "${PROJECT_SOURCE_DIR}/src")
  target_compile_features(bno08x PRIVATE c_std_23)
  target_compile_options(bno08x PRIVATE ${BASE_OPTIONS})

  enable_testing()
  add_subdirectory(tests)

  include(IDE)
endif()
