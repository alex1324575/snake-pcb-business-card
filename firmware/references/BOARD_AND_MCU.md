# Rev B board and STM32G030 firmware reference

Reviewed 2026-09-17. This note records an independent comparison of the exported KiCad netlist with ST documentation. It is not a hardware test report. The authoritative board input is `../../exports/SnakeBusinessCard.net.xml`; older design notes mentioning UART test points or an IICRST test point are superseded by that netlist.

## Exact board interface

U1 is **STM32G030K8T6**, LQFP32. Signal names below come from the actual exported board netlist, not an evaluation board pin assignment.

| U1 physical pad | GPIO/function | Board net | Firmware use |
|---|---|---|---|
| 1 | PB9 | NC | Leave analog |
| 2, 3 | PC14, PC15 | NC | Leave analog; no crystal fitted |
| 4 | VDD/VDDA | +3V3 | Shared digital/analog supply |
| 5 | VSS/VSSA | GND | Shared digital/analog ground |
| 6 | NRST | NRST / TP8 | External debug reset; R10 10k pull-up, C12 100n |
| 7 | PA0 | BTN_UP | Active-low input, R11 10k pull-up |
| 8 | PA1 | BTN_DOWN | Active-low input, R12 10k pull-up |
| 9 | PA2 | BTN_LEFT | Active-low input, R13 10k pull-up |
| 10 | PA3 | BTN_RIGHT | Active-low input, R14 10k pull-up |
| 11 | PA4 | BTN_START | Active-low input, R15 10k pull-up |
| 12 | PA5 | LED_EN | Push-pull output, initially low; R22 100k pull-down |
| 13, 14 | PA6, PA7 | NC | Leave analog |
| 15, 16, 17 | PB0, PB1, PB2 | NC | Leave analog |
| 18, 19 | PA8, PA9 | NC | Leave analog; no UART connection |
| 20 | PC6 | NC | Leave analog |
| 21, 22, 23 | PA10, PA11, PA12 | NC | Leave analog; no USB data connection |
| 24 | PA13 | SWDIO / TP6 | Preserve SWD configuration |
| 25 | PA14 | SWCLK / TP7 | Preserve SWD configuration; also BOOT0 function |
| 26 | PA15 | NC | Leave analog |
| 27, 28, 29 | PB3, PB4, PB5 | NC | Leave analog |
| 30 | PB6 | I2C_SCL | I2C1 SCL, **AF6**, open-drain; R20 2.2k pull-up |
| 31 | PB7 | I2C_SDA | I2C1 SDA, **AF6**, open-drain; R21 2.2k pull-up |
| 32 | PB8 | NC | Leave analog |

PB6/PB7 AF6 and package numbering were independently checked against [ST DS12991 Rev 6, June 2025, Figure 5 and Tables 12–14](https://www.st.com/resource/en/datasheet/stm32g030c6.pdf). This package has one combined supply pair. Set only the GPIO fields needed by the application; do not overwrite PA13/PA14 while configuring other pins.

| Test point | Net | Practical connection |
|---|---|---|
| TP1 | VBUS_5V | USB supply measurement |
| TP2, TP4 | +3V3 | TP4 → programmer VTref/sense |
| TP3, TP5 | GND | TP5 → programmer GND |
| TP6 | SWDIO | Programmer SWDIO |
| TP7 | SWCLK | Programmer SWCLK |
| TP8 | NRST | Programmer NRST |

Power the board through USB-C while flashing; connect the programmer's voltage-sense input to TP4, rather than treating an arbitrary programmer power output as VTref. USB-C is power only; this board has no USB bootloader data path. NFC has no MCU connection and remains independently provisioned.

## LED matrix and safe boot

U2 is IS31FL3737-QFLS4-TR. ADDR pad 36 is grounded, giving 7-bit address **0x50** (write address byte 0xA0). U2 SDB pad 38 is controlled by PA5. IICRST pad 39 has a 10k pull-down and no MCU/test-pad control. SYNC and INTB are unconnected. VCC, VIO and both PVCC pads use +3V3. R24 is **20k 1%** from R_EXT pad 30 to ground.

Matrix coordinate `(x, y)` is zero-based: `Dx = D(1 + x + 10*y)`, anode → `LED_COL(x+1)` / CS(x+1), cathode → `LED_ROW(y+1)` / SW(y+1). Thus D1 is row 1/column 1, D10 row 1/column 10, D81 row 9/column 1, and D90 row 9/column 10. Connected U2 CS pads are 15,16,17,18,20,21,22,23,24,25; SW pads are 1,2,3,5,6,7,8,9,10. CS11–12 and SW10–12 must remain disabled in the framebuffer mask.

Drive PA5 low before making it an output. Keep the display shut down while initializing current limits and clearing pixel state. A short SDB pulse is **not** an erase of retained driver registers. Every recovery must re-establish the driver's safe configuration, not assume power-on defaults. Hardware shutdown permits function-page access only; page-specific initialization order is covered by the driver reference.

For the fitted R24, the manufacturer's current relationship gives `Ipeak = 42 mA × GCC/256` and `Iaverage(pixel) ≈ Ipeak × PWM/256 ÷ 12.75`. As a conservative project choice, GCC=32 caps peak LED current at about 5.25 mA. At PWM=255, all 90 pixels together contribute about **36.9 mA average**, excluding driver/MCU/pull-up current. Normal Snake frames light fewer pixels. These are calculated nominal currents; verify actual USB input current, regulator temperature and brightness on the sample. Do not substitute full-scale GCC=255 as a generic initialization value. Equations and retention behavior: [Lumissil IS31FL3737 Rev G, 2024-10-28, pages 16, 20 and 24](https://www.lumissil.com/assets/pdf/core/IS31FL3737_DS.pdf), also archived in the board references.

## Clock, memory and startup

Use HSI16 with HSIDIV=0, AHB/APB division=1: SYSCLK/HCLK/PCLK = 16 MHz. RCC_CR power-on value 0x00000500 already represents HSI enabled/ready and **division by 1**; explicitly set clock selection for a deterministic entry. Do not assume the startup template programs the application's clock tree. I2C1 kernel selector RCC_CCIPR[13:12] accepts 00=PCLK or 10=HSI16; document the chosen source with TIMINGR. SysTick with CPU clock and reload **15999** gives nominal 1 ms. Flash: **0x08000000–0x0800FFFF**, 64 KiB; SRAM: **0x20000000–0x20001FFF**, 8 KiB; initial stack top **0x20002000**. Keep the vector section, initialize `.data`/`.bss`, and provide a real SysTick handler. A stock ST startup calls `SystemInit` before copying `.data`, so that function cannot depend on initialized globals. [RM0454 Rev 5](https://www.st.com/resource/en/reference_manual/rm0454-stm32g0x0-advanced-armbased-32bit-mcus-stmicroelectronics.pdf), [ST startup template](https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Source/Templates/gcc/startup_stm32g030xx.s), [ST system template](https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Source/Templates/system_stm32g0xx.c).

CMSIS names verified in the [official STM32G030 device header](https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Include/stm32g030xx.h), inspected 2026-09-17:

| Register / field | Bit(s) |
|---|---:|
| RCC_IOPENR GPIOAEN / GPIOBEN | 0 / 1 |
| RCC_APBENR1 I2C1EN | 21 |
| RCC_APBRSTR1 I2C1RST | 21 |
| RCC_APBENR1 DBGEN / PWREN | 27 / 28 |
| RCC_CR HSIDIV | 13:11 |
| RCC_CCIPR I2C1SEL | 13:12 |
| RCC_BDCR BDRST | 16 |
| RCC_CSR PWRRSTF / RMVF | 27 / 23 |
| DBG_APB_FZ1 DBG_IWDG_STOP | 12 |

G0 naming pitfalls: use `RCC->APBENR1`, not another STM32 family's `APB1ENR`; device debug registers use `DBG`, not `DBGMCU`. The implemented BSP uses an original minimal `stm32g030_min.h` containing only the verified register addresses and fields needed here; it does not vendor the ST header. Core register facts also use [ST PM0223](https://www.st.com/resource/en/programming_manual/pm0223-stm32-cortexm0-mcus-programming-manual-stmicroelectronics.pdf). Do not change boot/read-protection option bytes as part of routine game firmware.

## I2C timing calculation review

Proposed **TIMINGR=0x10F11F2F**, analog filter enabled, DNF=0, kernel=16 MHz. This is conservative Standard-mode timing: actual bus frequency is below 100 kHz.

| Field | Value | Nominal programmed interval |
|---|---:|---:|
| PRESC | 1 | tPRESC=125 ns |
| SCLDEL | 15 | 2,000 ns |
| SDADEL | 1 | 125 ns |
| SCLH | 31 | 4,000 ns before synchronization |
| SCLL | 47 | 6,000 ns before synchronization |

Independent arithmetic using [ST AN4235 Rev 2, August 2013, section 2.2.4](https://www.st.com/resource/en/application_note/an4235-i2c-timing-configuration-tool-for-stm32f3xxxx-and-stm32f0xxxx-microcontrollers-stmicroelectronics.pdf): assume rise ≤1,000 ns, fall ≤300 ns, analog delay 50–260 ns. At 16 MHz, SCL high minimum is 4.175 µs and low minimum 6.175 µs; maximum frequency with zero external edge delay is 96.62 kHz. Required SDADEL interval is 62.5–1,940 ns, containing the selected 125 ns. SCLDEL must be at least 1.25 µs, below the selected 2 µs.

For an additional conservative clock check, combining DS12991 Table 38 upper initial frequency, temperature drift and voltage drift gives 16.32936 MHz. The same calculation yields high ≥4.0918 µs, low ≥6.0515 µs, bus ≤98.59 kHz, SDADEL=122.48 ns within 66.28–1,945.04 ns, and SCLDEL=1.9597 µs. These are analytical bounds, not a CubeMX export or measured waveform. Confirm rise/fall and ACK behavior with a logic analyzer on the sample. The 2.2k pull-ups and PCB capacitance determine real edges.

Program timing with PE=0; use 7-bit addressing with the address shifted once into CR2.SADD. Count the register-address byte in NBYTES. Poll TXIS for each byte, then STOPF for completion. Every wait needs a bounded timeout, including BUSY/STOP/recovery. Keep slave addressing disabled. On persistent failure, pull SDB low, restore released open-drain bus lines and recover/reinitialize; do not drive SCL or SDA high push-pull.

## Errata and watchdog strategy

[ES0486 Rev 6, September 2023](https://www.st.com/resource/en/errata_sheet/es0486-stm32g030x6x8-device-errata-stmicroelectronics.pdf) matters directly:

- §2.2.10: short power cycles can corrupt RTC-domain state, potentially enabling LSCO on **PA2**, used here by LEFT. Before GPIO setup, when the power-reset flag is present, unlock backup-domain access and reset RCC_BDCR with BDRST, read back, then clear it. Save reset cause before clearing flags. The errata calls the power flag BORRSTF; this device header calls it **PWRRSTF**.
- §2.8.2: master-mode BERR may be spurious. Clear BERR and allow an otherwise healthy transfer to finish; handle genuine NACK, arbitration loss and timeout separately.
- §2.8.1: the input setup interval must exceed one I2C kernel clock period. At 16 MHz that is 62.5 ns; Standard-mode's 250 ns setup leaves margin.
- §2.8.3: do not enable own-address slave matching in this master-only application.

Project watchdog strategy: use IWDG with the independent LSI source, window disabled. A /64 prescaler and reload 999 yields nominal 2 s; for the documented 29.5–34 kHz LSI range it is about 1.88–2.17 s. Start it before potentially blocking display initialization, after the clock/tick system is usable. Feed only from the main loop after useful progress, never unconditionally from SysTick or an I2C polling loop. This makes a stuck bus or stalled main loop restart the MCU; R22 then pulls SDB low. Freeze IWDG while a debugger intentionally halts the core. Once started, IWDG cannot be stopped without reset; initialize it by the documented key/unlock/prescaler/reload/status/refresh sequence. [RM0454 independent watchdog chapter](https://www.st.com/resource/en/reference_manual/rm0454-stm32g0x0-advanced-armbased-32bit-mcus-stmicroelectronics.pdf).

## Bring-up observations still needed

Check first pixel D1 and corner pixels D10/D81/D90 for orientation, every button after 20 ms stable debounce, release-to-repress behavior, safe display blanking on bus failure, actual watchdog reset, USB unplug/replug, and SWD reconnect-under-reset. No firmware compile or host simulation can substitute for these board observations.
