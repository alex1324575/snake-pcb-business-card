# First-board bring-up

This procedure is for an assembled prototype. The project has digital design checks and host firmware tests; it has not been flashed or validated on physical hardware.

First close the pre-assembly items in [Component review](COMPONENT_REVIEW.md). Prepare a current-limited 5 V source with a suitable USB-C connection, multimeter, a 3.3 V-compatible SWD programmer such as ST-LINK, and [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html). Use an oscilloscope for rail transients and I2C rise/fall measurements; a logic analyzer helps inspect transactions.

## 1. Inspect before power

Check LED polarity, U1/U2 orientation, the QFN exposed-pad assembly, USB solder joints and obvious bridges. Leave C30 unpopulated. With power disconnected, inspect resistance between the supply rails and ground for an unintended short; interpret charging capacitors rather than treating a transient reading as a pass/fail threshold.

## 2. Identify the rear debug pads

Place the board **back side up, with rear text upright**. The five SWD pads read from left to right:

```text
TP8     TP7     TP6     TP5     TP4
RST     CLK     DIO     GND     3V3
```

![Back of the example board](images/back.png)

These are **1.0 mm diameter surface pads on 3.0 mm centers**, located 3.0 mm from the lower card edge. They are not through-holes or a 2.54 mm pin header. Use a aligned pogo-pin fixture or fine temporary wires.

| Board pad | Programmer connection |
|---|---|
| TP8 RST | NRST |
| TP7 CLK | SWCLK |
| TP6 DIO | SWDIO |
| TP5 GND | GND |
| TP4 3V3 | VTref / target-voltage sense |

Power the board through USB-C. TP4 senses the board's 3.3 V rail; connect it to the programmer's documented voltage-sense input. It is not an invitation to connect an unidentified programmer power output. TP1–TP3 are a separate group for 5 V, 3.3 V and ground measurements.

## 3. Check power and flash

1. Apply USB power and measure approximately 5 V at TP1 and 3.3 V at TP4, relative to ground. Disconnect and investigate unexpected current or heating.
2. Connect through ST-LINK/SWD in **Under Reset** mode, with NRST connected. Confirm STM32G030 and 64 KiB Flash.
3. Program and verify [SnakeBusinessCard_RevB_v1.0.0.hex](../firmware/release/SnakeBusinessCard_RevB_v1.0.0.hex), then reset. The HEX contains its addresses. A BIN must be written at **0x08000000**.
4. Keep boot option bytes and read protection unchanged for routine firmware installation.

Source builds, checksums and the equivalent CLI command are in the [firmware README](../firmware/README.md). Programming requires SWD; the USB connector has no data path.

## 4. Check every LED and button

Hold START while applying power. Diagnostic mode should scan D1 through D90 from the upper-left corner, across each row and then downward. Check the corners and every intermediate pixel for missing LEDs, polarity errors or row/column swaps. Press each direction to show its corresponding marker. Release START, then give a fresh short press to exit.

Normal startup shows the example initials **AX**, followed by the ready pattern. A short START press/release starts the game; directions turn the snake. START pauses/resumes; holding it for about 0.8 seconds returns to ready. Confirm collision, score, restart and power-on high-score reset. Full controls are in the firmware README.

## 5. Measure and record

| Test | Record |
|---|---|
| Rails and power | 5 V / 3.3 V readings, USB current, regulator/driver temperature |
| Display | All 90 pixels, orientation, brightness, ghosting |
| Inputs | All directions, START short/long press, debounce |
| I²C | Rise/fall <=300 ns at U2, ACKs, address 0x50, sustained operation |
| Recovery | Display blanking on an induced bus fault, bounded recovery, restart from ready |
| Watchdog / reset | Actual reset behavior, repeated power cycles, SWD reconnect under reset |
| Contact | Printed QR scan, tuned NFC read range, written URI and read-back |

Perform fault injection with suitable current limiting and instrumentation; do not short a supply to test software recovery. Keep the firmware's GCC limit at 32 while establishing the baseline. The LED-current calculation is not a substitute for measurement.

Record board revision, firmware SHA-256, supply, instruments, phone models and measured results. A blank test result means **not tested**. Follow [Contact and NFC](CONTACT.md) for the separate passive-tag workflow.
