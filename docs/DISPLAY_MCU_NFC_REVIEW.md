# MCU, display and NFC component review

> **Historical KiCad review.** The selections and geometry below describe the recorded Rev B baseline. Rev C uses the IS31FL3737B and KENTO KT-0603R selection, with a different layout and thermal path. Use the [current component register](COMPONENT_REVIEW.md), [BOM](revc/assembly/BOM.md) and [qualification record](revc/electrical/QUALIFICATION_PLAN.md).

Reviewed 2026-09-26 against the public Rev B netlist, native PCB, project footprints and current firmware. **These parts are selected for prototype procurement; this is a document and connectivity review, not an assembled-board approval.** No component was substituted and no PCB geometry was changed during this review.

## Exact purchasing identities and primary references

| References / quantity fitted | Manufacturer | Manufacturer part number | Reviewed manufacturer document and sections |
|---|---|---|---|
| U1 / 1 | STMicroelectronics | `STM32G030K8T6` | [DS12991, Rev 6, 20 June 2025](https://www.st.com/resource/en/datasheet/stm32g030c6.pdf): Figure 5 and Tables 12-14, operating conditions, Table 50, section 6.4, ordering information |
| U2 / 1 | Lumissil Microsystems | `IS31FL3737-QFLS4-TR` | [IS31FL3737, Rev G, 28 October 2024](https://www.lumissil.com/assets/pdf/core/IS31FL3737_DS.pdf): pp. 3-7 pinout/ordering/electrical limits, pp. 20-21 current control, pp. 25-28 layout/package |
| D1-D90 / 90 | Kingbright | `APT1608SURCK` | [DSAD0926 / 1203001764, V.22A, date printed 04/08/2023](https://www.kingbrightusa.com/images/catalog/SPEC/APT1608SURCK.pdf): pp. 1-3 polarity, land pattern, electrical ratings and assembly profile |
| U4 / 1 | STMicroelectronics | `ST25TN01K-AFH5` | [DS13433, Rev 5, 14 October 2025](https://www.st.com/resource/en/datasheet/st25tn01k.pdf): sections 1.2-2, 7-10; Figure 10 land example; Table 33 ordering code |

Use the complete MPN, including the package suffix. These references establish the selected device and package; they do not establish distributor stock or an approved substitute. In particular, an STM32 TSSOP part, a different IS31 matrix driver, an arbitrary red 0603 LED, and a wafer-form ST25TN are not interchangeable BOM replacements.

## U1: controller and interface

**Result: electrical selection and pin mapping reviewed; assembly and hardware behavior remain conditional.** The device supports a 2.0-3.6 V supply. The board uses 3.3 V on combined supply pin 4 and ground pin 5. The package is LQFP32, 7 x 7 mm with 0.8 mm lead pitch. The `K8T6` choice provides 64 KiB Flash and 8 KiB SRAM.

The exported netlist connects PA0-PA4 to the five buttons, PA5 to display enable, PB6/PB7 to I2C1, PA13/PA14 to SWD and pin 6 to reset. PB6/PB7 use AF6 in the firmware. Supply, reset and debug connections agree with the package pinout; unused pins have no board connections. Full pin-by-pin evidence is in [Board and MCU reference](../firmware/references/BOARD_AND_MCU.md).

The **native PCB** contains the KiCad `LQFP-32_7x7mm_P0.8mm` footprint with 1.50 x 0.50 mm lands. ST Figure 34 shows 1.20 x 0.45 mm lands. Package and pin compatibility are established, but the land geometry is not an exact match to ST's example. The assembler must review lead overlap, solder-mask separation and stencil apertures before assembly release.

At nominal 3.3 V, the MCU's specified CMOS output levels at 6 mA are at least 2.9 V high and at most 0.4 V low. The display enable load is only a 100 kohm pull-down plus input leakage. I2C is open-drain with 2.2 kohm pull-ups, approximately 1.5 mA per asserted line; no GPIO drives an LED directly. Check actual logic levels and startup ordering on the board.

## U2 and D1-D90: matrix compatibility

**Result: pinout, polarity and nominal operating point are compatible; brightness, temperature and timing need measurement.** U2's specified supply range is 2.7-5.5 V. Board VCC, VIO and both PVCC pins share 3.3 V. Its logic thresholds are 0.75 x VIO minimum high and 0.2 x VIO maximum low. All grounds, including exposed pad 41, are connected; ADDR is grounded, giving seven-bit address `0x50`. SDB has a pull-down and MCU control; IICRST has a pull-down. Unused matrix outputs, SYNC and INTB are unconnected.

The native PCB uses 0.715 x 0.22 mm peripheral lands at 0.4 mm pitch and a 3.4 mm square grounded exposed pad, matching the selected manufacturer's land dimensions. Sixteen 0.30 mm ground vias occupy the exposed-pad area. Nine paste windows are a project assembly choice. Stencil thickness and via filling/capping or other solder-wicking treatment remain assembler decisions; these cannot be inferred from a DRC pass.

The LED board footprints have two 0.8 mm square lands separated by 0.85 mm. Pad 1 is cathode to `LED_ROW1..9`; pad 2 is anode to `LED_COL1..10`. All 90 devices use the same MPN and orientation. Kingbright specifies 1.95 V typical and 2.5 V maximum forward voltage at 20 mA, 30 mA DC absolute maximum at 25 C, and a -40 to +85 C operating range. Its 185 mA pulse rating has separate pulse-width/duty conditions and is **not** used to justify this design. Observe the published temperature derating, reflow profile and MSL 3 handling.

### Current and voltage calculations

These calculations use the current firmware ceiling `IS31FL3737_MAX_GCC = 32`, R24 = 20 kohm, and the driver's nominal scan model. They are estimates at the datasheet test conditions, not guaranteed complete temperature-corner results.

| Calculation | Result / interpretation |
|---|---|
| Nominal peak source current: `42 mA x 32 / 256` | 5.25 mA per active LED |
| Nominal full-PWM average per pixel: `5.25 x 255 / 256 / 12.75` | 0.410 mA |
| All 90 pixels at full PWM | 36.9 mA average LED current, before IC/MCU/pull-up loads |
| Conservative scaling of the published 45 mA current limit (specified at GCC=255) with R24 at -1%: `45 x 32 / 255 / 0.99` | About 5.71 mA peak; approximately 40.1 mA aggregate average |
| Headroom screen: LED maximum VF plus published maximum source/sink drops | `2.5 + 0.20 + 0.35 = 3.05 V`; 0.25 V below nominal 3.3 V |

The current estimate is comfortably below the LED's 25 C DC limit without relying on its pulse rating. The 3.05 V screen uses headroom specified at substantially higher load than this design. It supports the 3.3 V design choice, but **does not prove full-temperature constant-current regulation**: LED VF limits are specified at 25 C, the VF footnote includes a 0.1 V tolerance, low-temperature VF rises, and regulator tolerance plus dynamic rail drops consume margin. Check cold/high-VF samples and the lowest measured rail; reduce current or revise the supply/display design if regulation or visibility is inadequate. Do not silently raise GCC to 255.

### I2C acceptance criterion

The existing MCU timing calculation includes a Standard-mode 1000 ns rise-time budget. The display driver's receiver specification is stricter: **verify rise and fall times of no more than 300 ns at U2**, even though the firmware clock is below 100 kHz. For a simple RC rise model, `tr = 0.8473 x Rpullup x Cbus`; 2.2 kohm implies roughly 160 pF maximum before resistor tolerance and measurement margin. Actual trace/input/probe capacitance has not been measured. A valid MCU timing-register calculation alone does not close this check.

## U4: passive NFC

**Result: MPN, package and isolated RF connectivity reviewed; NFC function remains unverified.** `ST25TN01K-AFH5` selects the UFDFPN5 package and nominal 50 pF internal tuning capacitance. The specified capacitance is 47.5-52.5 pF under its stated RF measurement conditions. This part harvests energy from the 13.56 MHz field; it has no 3.3 V supply or MCU interface.

The project maps AC0 to pad 3 and AC1 to pad 4, with pads 1, 2 and the central pad 5 unconnected. The central pad is **not ground**. The native lands are four 0.6 x 0.2 mm pads and one central 1.6 x 0.2 mm pad, consistent with Figure 10. The earlier footprint drawings were from Rev 2; the reviewed Rev 5 retains these connections and dimensions. Bottom-view package drawings must be interpreted in the PCB's top-view orientation.

The RF pins connect only to the printed antenna and DNP C30 tuning position. They are not coupled into the game supply. `NT1` is board copper and not a purchasable inductor. C30 stays unpopulated until measured tuning determines whether a capacitor is needed and, if so, its value. Selecting a convenient capacitor now would not validate the antenna.

The fresh-tag capability container advertises a 160-byte Type 2 data area. The public example URI fits, but the supplied payload has not been written to silicon. Verify tag identity, capability container and lock state before programming; do not overwrite UID, configuration or permanent lock data. Follow [Contact and NFC](CONTACT.md). There has been **no antenna tuning, VNA/impedance measurement, physical NFC write/read, or phone/read-range verification**.

## Required closure evidence

| Check | Evidence needed | Current state |
|---|---|---|
| U1 land and assembly | Assembler acceptance of the actual land/stencil geometry; inspection of first assembly | Open |
| MCU operation | SWD identification/programming, five buttons, reset, watchdog and repeated USB power cycles | Open |
| U2 assembly | Agreed via treatment/stencil; exposed-pad and peripheral joint inspection | Open |
| Display electrical | Measured 3.3 V ripple, LED peak/average current, all-pixel current and U2/LDO temperatures | Open |
| LED visual performance | Corner-pixel orientation, full 90-pixel walk, ghosting and indoor visibility | Open |
| I2C | U2-pin rise/fall <=300 ns, ACK reliability and injected bus-failure recovery | Open |
| Operating envelope | Low-rail and intended hot/cold tests; VF/current-regulation margin recorded | Open |
| NFC | Antenna measurement/tuning, URI write/read and multiple-phone tests with USB connected/disconnected | Open |

Digital tests and manufacturer-document checks justify a controlled prototype build. They do not close any of the physical checks above.
