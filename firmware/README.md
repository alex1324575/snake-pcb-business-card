# Snake firmware

Bare-metal C11 firmware for **Snake Business Card Rev B**, using an STM32G030K8T6 and an IS31FL3737 to drive a 10 × 9 LED matrix. The public example uses the identity **Alex Xu** and an **AX** startup graphic.

The firmware has passed native host tests, AddressSanitizer checks, an Arm target build, and automated checks against the published board. **It has not been flashed to or tested on an assembled board.**

## Play

| Input | Action |
|---|---|
| Power on | Show AX for approximately 0.8 seconds, then the idle animation |
| Short press and release START | Start, pause, resume, or restart |
| Direction button | Turn; one turn is accepted per movement interval |
| Hold START for approximately 0.8 seconds | Return to the idle screen |
| Hold UP while idle | Show the highest score since power-on |
| Hold START while powering on | Enter the LED and button diagnostic |

The head is brighter than the body, and food blinks. Eating food adds one point and one segment. Hitting a wall or the body ends the game. Filling all 90 cells wins with **87 points**. The result animation lasts approximately 0.9 seconds before a two-digit score appears. Movement starts at 320 ms per step, decreases by 10 ms per point, and stops accelerating at 110 ms.

Buttons require 20 ms of stable input. Simultaneous new direction presses are ignored; direct reversal is rejected. START activates on release for a short press. Resuming a paused game starts a full movement interval. Scores are kept in RAM and reset when power is removed.

## Build

Requirements: Python 3.9 or later and an Arm GNU toolchain providing `arm-none-eabi-gcc`, `objcopy`, `nm`, and `size`, with newlib-nano. No Python packages, code generators, HAL, or CMSIS source download is required.

From the repository root:

```sh
python firmware/tools/build.py --build
```

The script searches `ARM_GCC_BIN`, then PATH. On Windows it can also discover an existing Arduino15 Arm toolchain. To select a compiler explicitly:

```sh
python firmware/tools/build.py --build --gcc-bin /path/to/arm-toolchain/bin
```

The checked-in release uses **GNU Tools for Arm Embedded Processors 9-2019-q4-major, GCC 9.2.1**. Other compiler versions may generate different machine code and sizes. The GitHub workflow separately builds with Ubuntu's packaged Arm compiler; its results do not establish byte-for-byte reproducibility with the checked-in release.

### Artifacts

All outputs are under [`release/`](release/), with the stem `SnakeBusinessCard_RevB_v1.0.0`:

| File | Purpose |
|---|---|
| `.hex` | Intel HEX programming image with embedded addresses |
| `.bin` | Raw programming image; load at `0x08000000` |
| `.elf` | Executable with symbols and DWARF information for debugging |
| `.map` | Linker section, symbol, and library-object information |
| `manifest.json` | Compiler, source hashes, output hashes, and image checks |
| `build_report.txt` | Memory usage and per-function compiler stack reports |

Compiler inputs use repository-relative paths. C and assembler debug paths map the checkout directory to `.`, and toolchain headers map to `arm-toolchain`. Linker-map library paths use `<arm-toolchain>`. These substitutions keep local installation paths out of published artifacts without stripping debug symbols. Configure a debugger's source search path to the `firmware` directory if needed.

## Test

Native tests need GCC, Clang, or MSVC C++ Build Tools in C11 mode:

```sh
python firmware/tools/build.py --test-host --sanitize
python firmware/tools/check_board_contract.py
```

The first command runs the actual game, application, button, and display-driver code on the host. GCC and Clang enable AddressSanitizer and UndefinedBehaviorSanitizer; MSVC enables AddressSanitizer. The display tests use a simulated I²C callback. Reports are written to [`verification/`](verification/).

The second command reads the native PCB and exported XML netlist in this repository and checks their agreement with firmware constants. It validates pin assignments, LED coordinates and polarity, I²C address, and other electrical assumptions. It complements KiCad checks and physical testing. See the generated [board contract report](references/BOARD_CONTRACT_REPORT.md).

## Program the board

USB-C provides power only. Programming uses the rear SWD test points and a 3.3 V compatible ST-LINK.

| Board test point | Signal | Debug probe connection |
|---|---|---|
| TP4 | +3V3 | VTref / target voltage sense |
| TP5 | GND | GND |
| TP6 | SWDIO | SWDIO |
| TP7 | SWCLK | SWCLK |
| TP8 | NRST | NRST |

Power the board from USB-C. TP4 is the target voltage reference; connect it to the probe's voltage-sense input. Confirm the probe's pinout before connecting a power-output pin. See the repository's [bring-up guide](../docs/BRINGUP.md) for physical connections and acceptance checks.

In [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html), select ST-LINK, SWD, and Under Reset. Connect NRST, verify the target and its 64 KB Flash capacity, then download the HEX image with verification and reset. From the `firmware` directory, the corresponding command is:

```sh
STM32_Programmer_CLI -c port=SWD mode=UR -w release/SnakeBusinessCard_RevB_v1.0.0.hex -v -rst
```

On Windows the executable may be named `STM32_Programmer_CLI.exe`. No option-byte or read-protection changes are needed. The command follows [ST's CLI documentation](https://dev.st.com/stm32cube-docs/prog/2.23.0/en/docs/markup/CubeProg_Command_Lines.html). It has not been executed against a physical board for this release.

The NFC tag is an independent passive device. MCU firmware does not program its contact record.

## Implementation

| Source | Responsibility |
|---|---|
| `snake.c` | Game rules, food placement, turn queue, collision, and full-board victory |
| `buttons.c`, `app.c` | Debouncing, interface, timing, scoring, and diagnostics |
| `is31fl3737.c` | Page unlocking, sparse register mapping, PWM, and current ceiling |
| `board.c` | HSI16 clock, 1 ms SysTick, GPIO, I²C, watchdog, and bus recovery |
| `startup.S`, `stm32g030k8.ld` | Vector table, RAM initialization, and memory layout |
| `main.c` | Input loop, frame submission, display-fault handling, and recovery |

The driver uses seven-bit address `0x50`, PB6/PB7 AF6 open-drain I²C, and PA5 for SDB. Timing register `0x10F11F2F` targets a bus rate below 100 kHz. Global current is limited to 32; body, head, and food use PWM values 80, 180, and 255. Unused channels are disabled. SDB stays low until initialization has restored the driver's relevant registers.

Transactions time out after 30 ms. A display fault blanks the matrix and freezes the game. Recovery attempts occur approximately every 500 ms; successful recovery returns to idle and preserves the current power session's high score. The approximately two-second independent watchdog is refreshed by the main loop and freezes while the debugger halts the core. The BSP includes the applicable PA2 backup-domain and I²C BERR errata handling described in [BOARD_AND_MCU.md](references/BOARD_AND_MCU.md).

The supplied image occupies **4,988 bytes of Flash**. Static `.bss` is 36 bytes, with a 40-byte aligned static RAM boundary. This is not total RAM usage: the game and frame buffers are on the stack, `main` has a reported 408-byte stack frame, and the linker reserves at least 2,048 bytes for stack space. There is no heap allocation, Flash persistence, or USB data protocol.

## First-board diagnostics

Hold START while applying power. Release it once diagnostic mode starts; press and release START again to exit. A bright pixel advances from D1 to D90, left to right and top to bottom, every 120 ms. A complete sweep takes 10.8 seconds. Direction buttons illuminate their corresponding edge markers.

Before treating the design as hardware-validated, measure supply rails, current, temperature, LED output, and I²C edges; exercise every button, repeated power cycles, watchdog reset, and physical bus-fault recovery. See the [verification summary](verification/README.md) for the exact scope of completed tests and the [toolchain notices](references/TOOLCHAIN.md) for linked runtime components.
