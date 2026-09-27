# Power, connector, protection, and switch review

Completed **2026-09-27**, using manufacturer sources checked on 2026-09-26 and comparisons with the exported netlist, native PCB, and firmware. This is a component suitability review for a prototype, not a record of measurements on an assembled board. Procurement quantities and the complete passive-component selections are maintained in the project BOM.

## Disposition by component

| References | Manufacturer / exact orderable part | Package | Review result |
|---|---|---|---|
| U3 | Texas Instruments **TLV75533PDBVR** | DBV, SOT-23-5 | Pin assignment and normal 5 V operation checked; effective capacitance, thermal, and transient qualification required |
| U5 | Texas Instruments **TPD2E2U06DCKR** | DCK, SC70-3 | CC-channel assignment and operating voltage checked; system ESD untested |
| D91 | Texas Instruments **TPD1E10B06DYAR** | DYA, SOD-523 | VBUS connection and stand-off checked; does **not** guarantee the LDO's absolute-maximum voltage is respected during a transient |
| J1 | Same Sky **UJC-HP-3-SMT-TR** | Six electrical contacts, four shell anchors | Pin map and native land dimensions checked; sample mating and plated-slot process checks required |
| SW1-SW5 | Alps Alpine **SKQGABE010**, quantity 5 | 5.2 mm square, 1.5 mm high | Electrical mapping and load checked; via/flux assembly issue remains open |

The three TI orderable part pages identify the selected parts as **ACTIVE**: [U3](https://www.ti.com/product/TLV755P/part-details/TLV75533PDBVR), [U5](https://www.ti.com/product/TPD2E2U06/part-details/TPD2E2U06DCKR), and [D91](https://www.ti.com/product/TPD1E10B06/part-details/TPD1E10B06DYAR). This establishes lifecycle status, not distributor inventory or a reserved supply. The [Alps product page](https://tech.alpsalpine.com/e/products/detail/SKQGABE010/) identifies SKQGABE010 as **Standard**; the page's separate glossary lists other possible statuses and is not the part's status.

## U3: 3.3 V regulator

Source: [TI TLV755P, SBVS320D, Rev. D, September 2024](https://www.ti.com/lit/ds/symlink/tlv755p.pdf), sections 4, 5.1, 5.3-5.5, 7.1.1, 7.1.4, and 7.1.5.

| Datasheet item | Relevant specification |
|---|---|
| Operating input / enable | IN: 1.45-5.5 V; EN: 0-5.5 V |
| Absolute limits | IN and EN: -0.3 to 6 V; OUT: -0.3 to the lower of IN + 0.3 V and 6 V |
| Junction temperature | Recommended: -40 to +125 C; absolute maximum: +150 C |
| Current | Up to 500 mA, subject to dissipation; current limit is not a USB input-current limiter |
| Capacitors | Nominal 1 uF or greater at input and output; effective capacitance must remain above 0.47 uF after derating |
| DBV thermal data | JEDEC junction-to-ambient: 231.1 C/W; board-dependent, not a measured value for this card |

Native schematic and PCB agree: **1=VBUS_5V, 2=GND, 3=VBUS_5V (EN), 4=NC, 5=+3V3**. DBV has no exposed thermal pad. C1 and C2 connect directly across the corresponding rail and ground. Their positive pad centers are approximately 1.75 mm and 2.38 mm from the associated regulator pin centers; these distances do not characterize loop inductance or capacitor performance.

**Design assessment.** Nominal 5 V has 0.5 V headroom to the recommended input maximum and 1 V to the absolute maximum. Those are static margins, not a transient protection allowance. The procurement selection for C1/C2 is **TDK C1608X7R1V105K080AC, 1 uF, 35 V, X7R, 0603**. The passive review must close the actual DC-bias, tolerance, and temperature derating calculation; the 35 V rating alone is not that evidence.

A screening calculation at **5.25 V input and 100 mA output**, chosen as a test case rather than a measured board load, gives `(5.25 - 3.3) * 0.1 = 0.195 W`. Applying the JEDEC thermal figure gives a 45 C rise; a 25 C ambient would give approximately 70 C junction temperature. The card's two-layer copper geometry can change that result. The LED current estimate in [Hardware](HARDWARE.md#led-mapping-and-current) excludes MCU/driver overhead and tolerances, so it cannot establish a measured worst-case total current.

**Open checks:** measure input/output overshoot and undershoot, inrush, worst display-pattern current, and regulator temperature. Keep the SWD 3.3 V contact as voltage reference; externally driving +3V3 with USB absent can backfeed the regulator. Capture power-off behavior as well as startup.

## U5: CC protection

Source: [TI TPD2E2U06, SLLSEG9C, Rev. C, December 2019](https://www.ti.com/lit/ds/symlink/tpd2e2u06.pdf), sections 5, 6.1-6.5, and 10.

| Check | Specification and native-design comparison |
|---|---|
| Pinout | DCK pin 1=IO1, 2=IO2, 3=GND; board uses CC1, CC2, GND respectively |
| Operating range | 0-5.5 V, -40 to +125 C; passive CC resistor networks are within this normal voltage range |
| Stress rating | DCK: 5.5 A / 75 W for the specified 8/20 us pulse; not continuous power |
| ESD component rating | IEC contact +/-25 kV, air +/-30 kV; not a board certification |
| Signal loading | 1.5 pF typical capacitance; 10 nA maximum leakage at the stated 2.5 V test condition |

The netlist has separate CC1 and CC2 nets, each with its own 5.1 kohm pull-down. Neither channel connects to the MCU. The native footprint is `SOT-323_SC-70`, with exactly three pads. **DRL is a different five-pin package and is not a substitute.**

U5's two IO-pad centers are approximately 2.21 mm from their connector contacts. This proximity supports the intended placement but does not prove a low-inductance discharge path. Bench ESD testing must include both plug orientations, reset behavior, and permanent-damage checks.

## D91: VBUS ESD diode

Source: [TI TPD1E10B06, SLLSEB1G, Rev. G, August 2024](https://www.ti.com/lit/ds/symlink/tpd1e10b06.pdf), sections 4, 5.1-5.6, and 7.4.

| Check | Specification and native-design comparison |
|---|---|
| Pinout / package | Bidirectional two-terminal DYA/SOD-523; board pin 1=VBUS_5V, pin 2=GND |
| Operating range | +/-5.5 V, -40 to +125 C; nominal VBUS=5 V |
| Stress rating | 6 A / 90 W, specified 8/20 us waveform at 25 C |
| Breakdown | 6 V minimum |
| Clamp | Pin 1 to grounded pin 2: 10 V maximum at 1 A; 14 V maximum at 5 A, specified 8/20 us pulse |
| ESD component rating | IEC contact/air +/-30 kV |

**Protection coordination remains unverified.** The quoted 10 V clamp is already above U3's 6 V absolute maximum. Routing inductance can add overshoot, and negative excursions are also relevant to U3's -0.3 V limit. Therefore D91 is an ESD suppression component, not proof of protected-LDO survival and not protection against sustained overvoltage. No USB PD voltage is negotiated by this board.

The native SOD-523 lands are 0.6 x 0.7 mm, 1.4 mm center-to-center, and their net assignments match the schematic. D91 is approximately 2.67 mm from J1's nearer VBUS pad center. Confirm placement with the assembler and test the actual rail waveform; revise the input protection if it violates regulator ratings or fails system tests. The [DYA orderable-part page](https://www.ti.com/product/TPD1E10B06/part-details/TPD1E10B06DYAR) lists MSL 3 / 260 C / 168 hours; assembly handling must follow the delivered moisture-sensitivity label.

## J1: USB-C receptacle

Source: [Same Sky UJC-HP-3-SMT-TR datasheet](https://www.sameskydevices.com/product/resource/ujc-hp-3-smt-tr.pdf), retained manufacturer PDF **Rev. 1.03, 2026-08-31**, pages 1-2 and revision history. The live PDF fetch failed during this review; the search index exposed an older 2024 revision. The revision date above identifies the locally reviewed manufacturer document, not a claimed successful new download. The [manufacturer product listing](https://jp.sameskydevices.com/product/interconnect/connectors/usb-connectors/ujc-hp-3-smt-tr) also confirms the six-contact, power-only part.

Relevant connector ratings: 20 V DC; 3 A collectively through power/ground contacts, 1.5 A per such contact; 0.25 A for other contacts; -40 to +80 C. The manufacturer drawing specifies 0.8 mm PCB thickness. These connector ratings do not raise the assembled board's 5 V input limit.

Native-board verification:

| Contacts / geometry | Observed result |
|---|---|
| A5 / B5 | Separate USB_CC1 / USB_CC2 |
| A9 / B9 | Both VBUS_5V |
| A12 / B12 / S1-S4 | GND |
| Signal lands | 1.15 mm length; widths 0.9 mm GND, 0.8 mm VBUS, 0.7 mm CC |
| Four shell lands | 1.0 x 1.7 mm; 0.5 x 1.2 mm plated oval drills |
| Native board thickness | 0.8 mm |

The component opens outward from the right edge. The design uses approximately 0.4 mm front overhang; that is a project placement decision. Check a physical sample with the intended cable overmold, finished slot dimensions, shell protrusion below the board, and solder fillets before a production order. There are no data contacts or firmware-upload function.

## SW1-SW5: tactile switches

Sources: [Alps SKQGABE010 product page and land/circuit drawings](https://tech.alpsalpine.com/e/products/detail/SKQGABE010/); [product specification, October 2025](https://tech.alpsalpine.com/cms.media/SKQGAB_KQG_719_EN_cc4e10226e.pdf), sections 5-7, 9.1, and 10.

The specified resistive-load range is 1 V / 10 uA minimum to 12 V / 50 mA maximum, with -40 to +90 C operation. Initial bounce is at most 10 ms; the endurance specification allows 20 ms. Soldering instructions prohibit washing, limit hand soldering to 350 C for 3 s, and permit at most two reflow cycles.

**Electrical assessment.** Each button switches a 10 kohm pull-up to ground: approximately 3.3 V open and 0.33 mA closed, inside the stated switching-load range. Firmware uses a 20 ms stable-state filter in `firmware/include/buttons.h` and `firmware/src/buttons.c`.

The four physical terminals form two permanently connected pairs. The native KiCad footprint intentionally gives the two upper lands logical pad **1**, and the two lower lands logical pad **2**. All five instances use button-net pad 1 and ground pad 2. Their lands are 1.8 x 1.1 mm at local x=+/-3.1 mm, y=+/-1.85 mm, consistent with the manufacturer land drawing.

**Open assembly issue.** Section 10.3(4) of the product specification excludes through holes underneath or near the switch because flux can enter during assembly, particularly with subsequent wave soldering. The present PCB contains:

| Switch | Native PCB coordinates, mm | Finding |
|---|---|---|
| SW2 | Center (116.0000, 128.5000); BTN_DOWN via (116.0000, 126.6500) | Via is under the nominal 5.2 x 5.2 mm body |
| SW5 | Center (133.0000, 122.0000); BTN_START via (134.7666, 119.3913) | Via center is immediately outside the nominal body; still near the switch |

Do not mark this manufacturing constraint as passed from electrical DRC. Before assembly release, move these vias or obtain an assembler/manufacturer-approved process that addresses the deviation, including sealing where needed and avoiding flux-producing backside wave/flow soldering. Ordinary solder-mask tenting alone is not a documented qualification. Then confirm tactile action, solder quality, bounce, and flux exclusion on samples.

## Remaining closure evidence

| Item | Evidence required before claiming validation |
|---|---|
| C1/C2 stability margin | Exact capacitor MPN and conservative effective-capacitance calculation at operating bias and temperature |
| USB input behavior | Voltage/current captures during connection, removal, and representative cable/source combinations |
| Thermal margin | Measured worst-pattern current and stabilized regulator/driver temperatures with ambient recorded |
| Protection | Board-level ESD/transient results; compare regulator-pin waveform with its ratings |
| Connector | Sample mating, overmold clearance, plated-slot fit, and solder acceptance |
| Switch assembly | Closure of the two via findings and verification of the selected soldering/cleaning process |

These open items are retained explicitly so a completed procurement table cannot be mistaken for a tested production design.
