# AGENTS.md

Guidance for AI agents working in this repository.

## Project Summary

Firmware for NUCLEO-U545RE-Q (STM32U545RE) that reads orientation data from
a BNO08X IMU over I2C using the SHTP protocol. The I2C bus is abstracted
behind `src/i2c_bus.h` so the sensor driver (`src/bno08x.c`) can be unit-tested
on the host with Unity + CMock, without any ARM toolchain.

## Target Hardware

- MCU: STM32U545RE, LQFP64, Cortex-M33
- Board: NUCLEO-U545RE-Q
- IMU: BNO08X (CEVA / Hillcrest), I2C address 0x4A (SA0=GND)

## Key Constraints

- The I2C timing constant in `src/i2c_bus.c` (`0x00303D5B`) is calibrated for
  100 kHz with the default 4 MHz MSI clock (`RCC_MSIRANGE_4`). Recalculate
  with STM32CubeMX if the clock tree changes.
- PS0 and PS1 on the BNO08X must both be tied to GND to select I2C mode.
  Any other combination selects UART or SPI.
- `bno08x.c` must not include any HAL headers. All hardware access goes
  through `i2c_bus.h` so CMock can replace it in tests.

## Peripheral Map

| Peripheral | Role                                       |
| ---------- | ------------------------------------------ |
| I2C1   | BNO08X communication (PB6 SCL, PB7 SDA) |
| USART1 | printf output via ST-Link VCP (PA9 TX)   |

## Build Targets

| Command                                                          | Effect             |
| ---------------------------------------------------------------- | ------------------ |
| `cmake -B build/test && cmake --build build/test --target check` | Host unit tests    |
| `ninja -C build`                                                 | Firmware binary    |
| `ninja -C build flash`                                           | Flash via OpenOCD  |
| `ninja -C build debug`                                           | OpenOCD GDB server |

## Coding Conventions

- **Language:** C23
- **Trailing return type** for function signatures
  (e.g. `auto fn() -> void`)
- **4-space indentation**
- **No semicolons after closing braces** for namespaces/classes
- `auto` for obvious types (e.g. `auto main(...) -> int`)
- **East const** (e.g. `char const*` not `const char*`)
- **Trailing return type** for all function definitions, including
  operators (e.g. `auto operator=(T&&) noexcept -> T&`)
- **Public members first** in class declarations, private members at the
  bottom
- `<>` includes only for system headers (std, OS, etc.)
- `""` includes for third-party dependencies
  (e.g. `fmt`, `nlohmann/json`)
- **Naming:** `snake_case` for variables, functions, and classes
- **Naming:** `SCREAMING_SNAKE_CASE` only for macros and constants
- Include order:
  1. C++ standard library headers (`<chrono>`, `<vector>`, etc.)
  2. _(blank line)_
  3. C standard library headers (`<stdlib.h>`, `<string.h>`, etc.)
  4. _(blank line)_
  5. OS-specific headers (Windows API, POSIX, etc.)
  6. _(blank line)_
  7. Third-party dependencies (`"fmt/core.h"`, etc.)
  8. _(blank line)_
  9. Local/project headers
- No STM32CubeMX. Peripheral init is written by hand in source files.
- No dynamic memory allocation.
- `src/bno08x.c` depends only on `src/i2c_bus.h` — keep it that way.

## Shell Scripts

- Always use `#!/bin/sh` shebang for shell scripts
- Scripts must be POSIX compliant (no bashisms)
- When providing commands to users:
  - Windows/PowerShell: use `` ` `` for line continuation
  - Unix/Linux/macOS: use `\` for line continuation

## Commit Messages

- Follow the 50/72 rule:
  - Subject line: max 50 characters
  - Body lines: wrapped at 72 characters
- Use conventional commit prefixes (`feat:`, `fix:`, `docs:`, `chore:`,
  etc.)
- Separate subject from body with a blank line
- Do **not** add yourself as a co-author (`Co-Authored-By:` trailers are
  forbidden)

Example:

```
feat: add stopwatch timer

Replace Hello World with a live stopwatch that prints elapsed time
in HH:MM:SS.mmm format, updating every 10ms with color output.
```

## Documentation (Markdown)

- Wrap normal text and lists at **max 80 columns** (for readability in
  terminals and editors).
- **Exceptions**: Tables and code blocks can exceed 80 columns when
  formatting requires it (e.g. trees, alignment).
- Use standard Markdown: `**bold**`, `` `inline code` ``, `##` headings,
  `-` or numbered lists, fenced code blocks with language hints
  (` ```c `, ` ```sh `).
- Keep examples concise, up-to-date, and self-documenting.
- Do not use em dashes (`--`). Use a colon or rewrite the sentence.
- Each shell command gets its own fenced code block; do **not** combine
  multiple commands into one block. Precede each block with a short
  plain-text label describing what the command does:
- README.md: hardware wiring, pin assignments, build instructions.
  Keep it up to date when changing peripheral assignments.
- This file (`AGENTS.md`) follows its own rules.
