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

| Peripheral | Role                                    |
| ---------- | --------------------------------------- |
| I2C1       | BNO08X communication (PB6 SCL, PB7 SDA) |
| USART1     | printf output via ST-Link VCP (PA9 TX)  |

## Build Targets

| Command                                                          | Effect             |
| ---------------------------------------------------------------- | ------------------ |
| `cmake -B build/test && cmake --build build/test --target check` | Host unit tests    |
| `ninja -C build`                                                 | Firmware binary    |
| `ninja -C build flash`                                           | Flash via OpenOCD  |
| `ninja -C build debug`                                           | OpenOCD GDB server |

## TDD Workflow

This project follows Red-Green-Refactor. All changes to testable
source files under `src/` should be test-driven: write a failing
test first, then implement.

### Adding a new module

#### 1. Declare in the header

Create `src/<module>.h` with the public prototype:

```c
bool module_do_thing(void);
```

#### 2. Write a failing test

Create `tests/test_<module>.c`. Set CMock expectations for any
dependency calls the function will make, then assert the result:

```c
void test_do_thing_succeeds(void) {
    dep_fn_ExpectAndReturn(arg, true);
    TEST_ASSERT_TRUE(module_do_thing());
}
```

#### 3. Register the test in `tests/CMakeLists.txt`

```cmake
add_executable(test_module test_module.c)
target_include_directories(test_module PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(test_module
    PRIVATE module Unity::Unity CMock::CMock)
target_compile_features(test_module PRIVATE c_std_23)
cmock_generate_mock(test_module "${CMAKE_SOURCE_DIR}/src/dep.h")
add_test(NAME test_module COMMAND test_module)
list(APPEND TEST_TARGETS test_module)
```

#### 4. Add a stub, confirm RED, implement, confirm GREEN

Stub `src/<module>.c` with a dummy return value, run tests to confirm
the failure, implement the real logic, then confirm it passes:

```sh
cmake -B build/test && cmake --build build/test --target check
```

### Mocking a dependency

To mock a header, add `cmock_generate_mock` to the test target in
`tests/CMakeLists.txt`:

```cmake
cmock_generate_mock(test_module "${CMAKE_SOURCE_DIR}/src/dep.h")
```

CMock generates `Mockdep.h` into the build directory. Include it and
use the generated API in your test:

```c
#include "Mockdep.h"

void setUp(void)    { Mockdep_Init(); }
void tearDown(void) { Mockdep_Verify(); Mockdep_Destroy(); }

void test_something(void) {
    dep_fn_Expect(expected_arg);
    module_do_thing();
}
```

`_Expect` records the expected argument. `_Verify` confirms the
expected call count was met.

## Coding Conventions

Rules marked **[C++ only]** apply only to `.cpp` and `.hpp` files.

- **Language:** C23; C++23 for `.cpp`/`.hpp` files
- **4-space indentation**
- **East const** (`char const*` not `const char*`)
- **Trailing return type** **[C++ only]**
  (`auto fn() -> void`, including operators:
  `auto operator=(T&&) noexcept -> T&`)
- **`auto` for obvious types** **[C++ only]**
- **No semicolons after closing braces** for namespaces/classes
  **[C++ only]**
- **Public members first** in class declarations **[C++ only]**
- `<>` includes for system/standard headers; `""` for third-party
  and local headers
- **Naming:** `snake_case` for variables, functions, and classes
- **Naming:** `SCREAMING_SNAKE_CASE` only for macros and constants
- No STM32CubeMX. Peripheral init is written by hand in source files.
- No dynamic memory allocation.
- `src/bno08x.c` depends only on `src/i2c_bus.h` — keep it that way.

Include order for `.c`/`.h` files:
1. C standard library headers (`<stdlib.h>`, `<string.h>`, etc.)
2. _(blank line)_
3. OS-specific headers (Windows API, POSIX, etc.)
4. _(blank line)_
5. Third-party headers (`"unity.h"`, etc.)
6. _(blank line)_
7. Local/project headers

Include order for `.cpp`/`.hpp` files:
1. C++ standard library headers (`<chrono>`, `<vector>`, etc.)
2. _(blank line)_
3. C standard library headers (`<stdlib.h>`, `<string.h>`, etc.)
4. _(blank line)_
5. OS-specific headers (Windows API, POSIX, etc.)
6. _(blank line)_
7. Third-party dependencies (`"fmt/core.h"`, etc.)
8. _(blank line)_
9. Local/project headers

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
- Use conventional commit prefixes (`feat:`, `fix:`, `docs:`, `chore:`, `ci:`,
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

## Behavioral Guidelines

Reduce common LLM coding mistakes. These guidelines bias toward caution
over speed. For trivial tasks, use judgment.

### 1. Think Before Coding

Don't assume. Don't hide confusion. Surface tradeoffs.

Before implementing:

- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them, don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

### 2. Simplicity First

Minimum code that solves the problem. Nothing speculative.

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?"
If yes, simplify.

### 3. Surgical Changes

Touch only what you must. Clean up only your own mess.

When editing existing code:

- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it, don't delete it.

When your changes create orphans:

- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

Every changed line should trace directly to the user's request.

### 4. Goal-Driven Execution

Define success criteria. Loop until verified.

Transform tasks into verifiable goals:

- "Add validation" means: write tests for invalid inputs, then make
  them pass.
- "Fix the bug" means: write a test that reproduces it, then make it
  pass.
- "Refactor X" means: ensure tests pass before and after.

For multi-step tasks, state a brief plan:

```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria
("make it work") require constant clarification.
