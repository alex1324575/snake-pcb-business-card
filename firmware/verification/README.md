# Firmware verification

This directory records automated checks of the public **Alex Parker / AP** example firmware. The board has **not been programmed or tested physically**.

## Completed locally

| Check | Result and scope |
|---|---|
| Arm target build | GCC 9.2.1, Cortex-M0+, C11, warnings treated as errors |
| Native tests | All three executables passed with MSVC C11 `/W4 /WX` and AddressSanitizer |
| Snake engine | Eight groups, 4,096 initial seeds, food placement, turn queue, reverse rejection, collisions, vacating tail, growth, speed, and full-board victory |
| Complete game simulation | 1,896 steps, 87 points, 90 occupied cells, stable victory state |
| Application and buttons | Nine groups covering debounce, uptime wrap, short and long presses, pause, diagnostics, simultaneous directions, score rendering, display recovery, and victory |
| LED driver | Page unlocking, reserved addresses, retained-state initialization, all 90 pixels, disabled unused channels, and current limit |
| Bus fault injection | Failure at each of 68 initialization, 20 frame, and three current-setting transaction positions; immediate SDB shutdown and recovery through reinitialization |
| Board contract | 393 checks passed; all 90 LED positions checked; five deliberately invalid inputs rejected |
| Image integrity | Valid initial stack, Thumb reset and SysTick vectors, Flash/RAM bounds, HEX checksums, and exact HEX/BIN agreement |
| Memory | 4,988-byte Flash image, 36-byte `.bss`, 40-byte aligned static RAM boundary; 408-byte `main` stack frame and at least 2,048 bytes reserved for stack |
| Publication paths | Full ELF debug information retained with checkout/toolchain prefix mappings; distributable artifacts checked for local build paths |
| Repeated local build | A second build with the same compiler produced byte-identical ELF, HEX, BIN, MAP, manifest, and build report |

[`test_report.txt`](test_report.txt) contains the actual native test commands and output, with repository-relative paths. [`test_report.json`](test_report.json) records test source hashes and sanitizer selection. Target compiler flags, input hashes, output hashes, and image checks are in [`../release/manifest.json`](../release/manifest.json). The generated [board contract report](../references/BOARD_CONTRACT_REPORT.md) records its hardware inputs and their hashes.

## Reproduce

From the repository root:

```sh
python firmware/tools/build.py --test-host --sanitize --build
python firmware/tools/check_board_contract.py
```

The GitHub Actions workflow runs Linux GCC tests with AddressSanitizer and UndefinedBehaviorSanitizer, builds for Arm, and repeats the board contract check. Its status should be read from the actual workflow run; the local results above do not claim a GitHub run has completed.

## Boundaries

Native tests execute the portable C modules. The display driver's callback is simulated. Register accesses in `board.c`, startup assembly, and the main loop have been compiled and reviewed but have not executed on an emulator or a real STM32 for this release.

I²C waveforms, electrical startup behavior, visible brightness, current, temperature, tactile behavior, hot-plug behavior, watchdog resets, and physical bus recovery remain unmeasured. Static RAM alone does not describe peak runtime RAM usage. Hardware acceptance steps are in the [firmware guide](../README.md).
