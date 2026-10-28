# Rev C first-board bring-up

This procedure applies to **Altium Rev C / SC-REVC-LCSC-P1**. No MCU execution or physical test result is recorded. Confirm the board, installed BOM and drawings match Rev C before applying power; the historical KiCad rear-pad drawing does not apply.

Use the [assembly manual](revc/assembly/ASSEMBLY_MANUAL.md) and the complete [P01–P14 qualification record](revc/electrical/QUALIFICATION_PLAN.md). Prepare a current-limited 5 V USB source, multimeter, microscope, 3.3 V-compatible SWD programmer and oscilloscope with short ground springs. NFC tuning requires a suitable RF fixture. Record board serial, installed MPNs/lots, firmware hash, instruments, conditions, readings and trace filenames.

## 1. Inspect without power

Check all 90 LED cathodes, IC pin-1 marks, D91 polarity, MCU leads, QFN joints and USB shell/contact joints. D91 must be **PESD5Z6.0,115, K1 to VBUS_5V and A2 to GND**. U3 must be TPS7A2433DBVR. Keep C30 unpopulated and U4's NC pads isolated.

Check VBUS–GND and +3V3–GND resistance in both probe polarities and observe capacitor charging. A persistent near-zero reading requires diagnosis. Inspect hidden QFN joints with the selected qualified method; visible leads alone are insufficient.

## 2. Connect SWD on both faces

Rev C's contacts are distributed across both faces. The placement register uses the unmirrored native Top coordinate frame; the Bottom drawing mirrors X for a direct underside view. Native board-local coordinates are `(X − 100 mm, Y)`.

| Programmer signal | PCB contact | Face | Native X / Y, mm |
|---|---|---|---|
| SWDIO | TP6 | Bottom | 137.9982 / 12.2936 |
| SWCLK | TP7 | **Top** | 142.6464 / 9.4996 |
| NRST | TP8 | Bottom | 157.9880 / 2.9972 |
| Target-voltage sense | TP4, +3V3 | Bottom | 145.9992 / 1.5240 |
| Ground | TP5 | Bottom | 148.9964 / 2.9972 |

Use [the current drawings](revc/assembly/ASSEMBLY_REFERENCE.html) to locate each contact. A two-sided pogo fixture or short temporary wires must support the board without pressing on LEDs or the NFC area. These positions are not a 2.54 mm header or the historical five-pad row.

Power through controlled USB input. Connect TP4 only to the programmer's documented **target-voltage sense input**; disable programmer target-power output. Do not externally energize +3V3 while VBUS is absent. U3 does not limit reverse current. Connect GND first and disconnect it last.

## 3. Establish the power rails

1. Begin with regulated 5 V, display disabled and a 20 mA current limit. This is a fault-detection starting point, not a guaranteed operating-current specification. If current limiting occurs, power off and diagnose before increasing the limit.
2. Measure at U3 IN/OUT and U1 VDD. The steady +3V3 acceptance band is **3.135–3.465 V**, with no sustained oscillation, unexplained heating or current-limit event.
3. Capture startup/shutdown at 4.75, 5.00 and 5.25 V. Use short probe connections and record the actual waveform; a multimeter average cannot establish transient safety.
4. Keep the prototype envelope at 10–40 °C ambient, firmware GCC ≤32 and the project's 100 mA input-current screen pending qualification.

C1/C2 require effective capacitance **≥0.50 µF** after the declared bias, tolerance, temperature, measurement uncertainty and aging allowances. A nominal 1 µF label or typical-data curve does not close this acceptance item.

## 4. Identify, program and recover the MCU

Use a documented 3.3 V-compatible programmer such as ST-LINK and [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).

1. Start SWD at 100 kHz. Use connect-under-reset with NRST connected if needed.
2. Confirm STM32G030 identity and 64 KiB Flash.
3. Program and verify [SnakeBusinessCard_RevB_v1.0.0.hex](../firmware/release/SnakeBusinessCard_RevB_v1.0.0.hex), then reset. Original release filenames are retained; the unchanged firmware's native Rev C connection contract was checked separately. HEX includes its address; BIN must be written at **0x08000000**.
4. Keep boot option bytes and read protection unchanged for routine installation. Record the exact image hash and programming log.

The [firmware guide](../firmware/README.md) provides tool setup and image checksums. Its older physical rear-pad drawing is historical; use the Rev C table above. USB has no firmware-upload data path.

## 5. Check pixels, inputs and game behavior

Hold START during power-up to enter diagnostics. Check D1 through D90 individually from upper-left, across each row, then downward. Confirm no reversal, missing pixel, row/column swap or visible ghosting. Test all five buttons. Release START and give a fresh short press to exit.

Normal startup shows **AX**. START begins/pauses/resumes, a long press returns to idle, and the direction buttons steer. Confirm collision, score, restart and power-session high score. Keep GCC ≤32 in diagnostics as well as normal play.

## 6. Execute and record qualification

P01–P14 cover inspection, startup/load response, I²C timing, 30-minute worst-pattern thermal/current tests, pixels/buttons, fault/watchdog recovery, repeated SWD access, hot-plug, NFC, positive/negative system stress and regulator reverse bias. Follow their exact conditions and acceptance criteria in the [qualification record](revc/electrical/QUALIFICATION_PLAN.md).

Do not short a rail to simulate a bus fault. Use a controlled fixture and instrumentation. P14 must check output-to-input reverse stress during fast VBUS removal; steady operation does not close it. Keep C30 DNP until measured NFC tuning supports a selected value, then verify the example URL on at least one iOS and one Android phone.

All 14 results currently remain **NOT TESTED**. Enter measured PASS/FAIL evidence against the exact assembled variant, resolve failures and repeat affected tests after rework before accepting the first article.
