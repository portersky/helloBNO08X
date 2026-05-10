# helloBNO08X

BNO08X IMU sensor driver for STM32U545, with a TDD-friendly architecture
using Unity + CMock. The I2C bus is abstracted behind a thin HAL-free
interface so the sensor logic can be unit-tested on the host without any
ARM toolchain.

## Hardware

| Component  | Part                                  |
| ---------- | ------------------------------------- |
| MCU Board  | NUCLEO-U545RE-Q (STM32U545RE, LQFP64) |
| IMU Sensor | BNO08X (CEVA / Hillcrest)             |

## BNO08X Electrical Characteristics

| Parameter           | Min  | Typ  | Max | Unit |
| ------------------- | ---- | ---- | --- | ---- |
| Supply voltage      | 1.71 | 3.3  | 3.6 | V    |
| I2C clock (FM)      | -    | 400  | 400 | kHz  |
| I2C address (SA0=0) | -    | 0x4A | -   | -    |
| I2C address (SA0=1) | -    | 0x4B | -   | -    |

## I2C Wiring

```text
NUCLEO                BNO08X
──────                ──────
PB6 (I2C1_SCL) ────── SCL   (H_SCL, pin 19)
PB7 (I2C1_SDA) ────── SDA   (H_SDA, pin 20)
3V3            ────── VIN
GND            ────── GND
GND            ────── SA0   (I2C address: 0x4A)
GND            ────── PS0   (protocol select: must be GND for I2C)
GND            ────── PS1   (protocol select: must be GND for I2C)
```

Both lines need 4.7 kΩ pull-ups to 3.3V. The NUCLEO 3V3 rail is within
the BNO08X supply range. No level shifting is required.

## UART Log (ST-Link VCP)

`printf` output is routed to **USART1 TX on PA9** (ST-Link Virtual COM
Port). Connect with any serial terminal at **115200 8N1**. On boot you
should see:

```
helloBNO08X starting
BNO08X ready  sw=4.3.0  reset_cause=1
```

On Windows the VCP appears as a `COMx` port in Device Manager. On Linux
it is `/dev/ttyACM0` or `/dev/ttyUSB0`.

> **Note**: The I2C timing constant in `src/i2c_bus.c` is calibrated for
> the default 4 MHz MSI clock (`RCC_MSIRANGE_4`). Recalculate with
> STM32CubeMX if you change the clock tree.

## Project Layout

```text
helloBNO08X/
├── cmake/          ARM toolchain, MCU config, OpenOCD targets
├── deps/           FindUnity, FindCMock, Platform, Flags, Coverage, IDE
├── lib/            CMSIS, STM32U5 HAL, BSP, startup, linker scripts
├── src/
│   ├── i2c_bus.h   HAL-free I2C interface (the mockable seam)
│   ├── i2c_bus.c   Real HAL I2C implementation (firmware only)
│   ├── bno08x.h    BNO08X sensor interface
│   ├── bno08x.c    BNO08X SHTP driver (depends only on i2c_bus.h)
│   ├── uart_log.h  USART1 printf redirect
│   ├── uart_log.c  USART1 TX on PA9 → ST-Link VCP
│   ├── main.c      Firmware entry point
│   ├── syscalls.c  Newlib stubs (__io_putchar hook)
│   └── sysmem.c    Heap implementation
└── tests/
    ├── cmock_config.yml    Enables :ignore_arg and :return_thru_ptr plugins
    ├── CMakeLists.txt      Unity + CMock test registration
    └── test_bno08x.c       8 tests covering bno08x_init and get_product_id
```

## Build

### Tests (host, no hardware needed)

Requires: CMake ≥ 3.25, a C compiler, Ruby (for CMock code generation).

Configure:
```sh
cmake -S . -Bbuild/test -GNinja
```

Run tests:
```sh
ninja -C build/test check
```

Example output:

```
Unity test run 1 of 1
........

OK (8 tests, 8 ran, 8 passed, 0 failed, 0 ignored)
```

### Firmware (ARM)

The ARM GNU toolchain is downloaded automatically into `.tools/` on first
configure. Requires: CMake ≥ 3.25, Ninja, OpenOCD (bundled via
FetchContent).

Configure:
```sh
cmake -S . -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE="cmake/arm-none-eabi-toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug
```

Build:
```sh
ninja -C build
```

Flash:
```sh
ninja -C build flash
```

`compile_commands.json` is written to `build/`. Point your editor or
clangd config there directly. Most IDEs (Zed, VS Code + clangd) accept a
path to the build directory.

## Debugging

Start the OpenOCD GDB server (leave this terminal open):
```sh
ninja -C build debug
```

In a second terminal, connect GDB:
```sh
ninja -C build gdb
```

### Zed

Create `.zed/settings.json`:

```json
{
  "dap": {
    "GDB": {
      "binary": ".tools/arm-gnu-toolchain-15.2.rel1-mingw-w64-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb"
    }
  }
}
```

Create `.zed/debug.json`:

```json
[
  {
    "label": "Debug firmware (OpenOCD)",
    "adapter": "GDB",
    "request": "launch",
    "program": "$ZED_WORKTREE_ROOT/build/firmware.elf",
    "gdb_args": [
      "-ex",
      "target remote :3333",
      "-ex",
      "load",
      "-ex",
      "monitor reset init"
    ]
  }
]
```

Start OpenOCD first, then launch from the Zed debug panel:
```sh
ninja -C build debug
```
