# Rev C power ECO implementation and qualification

Selected 27 October 2026; native implementation verified 28 October 2026. The saved public Rev C design now specifies **U3 Texas Instruments TPS7A2433DBVR** and **D91 Nexperia PESD5Z6.0,115** in the native schematic, schematic library, PCB metadata and controlled assembly BOM. C1/C2 remain Samsung CL10B105KA8NNNC for prototype evaluation, subject to the effective-capacitance acceptance below. **The CAD ECO is applied; all physical qualification remains NOT TESTED.**

| Reference | Exact replacement | Procurement reference | Existing PCB connection |
|---|---|---|---|
| U3 | TPS7A2433DBVR, fixed 3.3 V, DBV 5-pin SOT-23 | [LCSC C2866134](https://www.lcsc.com/product-detail/C2866134.html) | 1 IN / VBUS_5V; 2 GND; 3 EN / VBUS_5V; 4 NC / open; 5 OUT / +3V3 |
| D91 | PESD5Z6.0,115, unidirectional, SC-79/SOD523 | [LCSC C461166](https://www.lcsc.com/product-detail/C461166.html) | **1 K / VBUS_5V; 2 A / GND**. Cathode marking faces pad 1. |

TI lists TPS7A2433DBVR as active. The regulator has 20 V IN/EN absolute limits, an 18 V operating ceiling, 200 mA capability, and maximum 160 mV dropout at 100 mA. Its capacitor table assumes 50% derating: nominal CIN 1 uF and COUT 1-100 uF correspond to approximately 0.5 uF minimum effective capacitance. EN can connect to IN; NC can remain open. Reverse current is not limited, and disabled output is high impedance. [TI datasheet SBVS386E, sections 5, 6.1, 6.3, 6.5, 7.3 and 8.1](https://www.ti.com/lit/ds/symlink/tps7a24.pdf), [exact order code](https://www.ti.com/product/TPS7A24/part-details/TPS7A2433DBVR).

The Nexperia device has 6 V standoff, maximum 13 V clamp at 5 A and 18 V at 10 A, both for 8/20 us pulses at 25 C. Pin 1 is cathode; pin 2 is anode. The 16 September 2026 manufacturer sheet explicitly identifies SOD523. [Nexperia datasheet, tables 1, 2 and 6](https://assets.nexperia.com/documents/data-sheet/PESD5Z6_0.pdf). LCSC identifies this exact manufacturer/order code, not a same-name replacement from another manufacturer. Inventory is subject to checkout verification.

## Package compatibility measured against the saved Rev C

The native electrical inventory and PcbLib binding establish the following existing geometry, accounting for U3's 90 degree placement:

| Feature | Saved Rev C | Compatibility assessment |
|---|---|---|
| U3 lead pitch | 0.9500 mm | Matches DBV lead pitch. |
| U3 pad size | 1.3250 x 0.6000 mm | 1.325 mm dimension follows each lead's long axis. |
| U3 opposing pad-row spacing | 2.2750 mm | Existing copper extends 0.4750-1.8000 mm from the body center along each lead axis. |
| U3 retained land extent | 3.6000 mm overall | TI's example uses 1.1 x 0.6 mm pads on 2.6 mm row centers, 3.7 mm overall. The existing longer pads extend further toward the heel and retain outer toe coverage; they are an alternate land pattern, not an exact copy of TI's example. |
| D91 pads | 0.6000 x 0.7000 mm; 1.4000 mm center spacing | Existing SOD523 geometry; pin mapping needs no net or orientation change. |

TI DBV lead-terminal span is 2.6-3.0 mm. Existing U3 copper reaches 1.8 mm from center on each side, beyond the maximum 1.5 mm terminal reach. Its 0.6 mm pad width accommodates 0.3-0.5 mm lead width and 0.95 mm pitch. This is a dimensional solder-land compatibility review; solder fillets and placement tolerance still need first-article inspection. [TI DBV0005A package and land drawing](https://www.ti.com/lit/ds/symlink/tps7a24.pdf). No pad resizing or rule change is required for this ECO.

Measurement source: the retained geometry baseline summarized in the PCB metadata audit from baseline PCB SHA-256 `79b43f05ff88d4faba876225f47f78b5072b034843425330ed8078656210dd0e`. The saved [PCB metadata audit](../../../verification/revc/PCB_METADATA_AUDIT.json) confirms that the applied ECO preserves pad geometry, vias, placed-component geometry/identity, net identity/order and the frozen rule stream. The comparison accounts only for serialized primitive order and component-owner index rebinding; it does not waive any pad field or flag.

## Protection decision and its boundary

The original 10 V specified positive clamp exceeded TLV755P's 6 V IN/EN limit. The applied U3 replacement corrects that specific positive voltage-rating mismatch in the saved design and selected BOM. The applied D91 replacement also changes the negative-pulse path from bidirectional avalanche to forward conduction. At the Nexperia 5 A test point, 13 V is 7 V below the replacement U3's positive absolute limit; at 10 A the arithmetic margin is only 2 V. These differences are a design screen at the stated pulse and temperature, not available layout-overshoot margin for every ESD waveform.

The replacement still has a -0.3 V IN/EN absolute limit. A unidirectional silicon TVS does **not** guarantee that negative stress stays above -0.3 V. The device's IEC withstand rating describes survival of the TVS; it does not establish survival of the assembled regulator or MCU. Keep product ESD/EMC qualification open. Measure U3 IN/EN/OUT during the specified positive and negative system tests, including ground inductance and initial overshoot. A failed trace requires additional input isolation/clamping and another CAD revision; it cannot be accepted by citing a TVS's nominal working voltage.

## Capacitors, reverse current and operating limit

Use **Ceff >=0.50 uF at C1 and C2**, with measurement uncertainty and a declared aging allowance deducted, as the replacement-regulator prototype acceptance floor. The retained Samsung typical-data screens are 0.6100 uF at 5.5 V input and 0.6993 uF at 3.3 V output, including the previous tolerance/temperature factors. They exceed that floor by 22.0% and 39.9% respectively before the unclosed aging/curve-guarantee allowances. They are not manufacturer guaranteed combined-corner minima. The controlled [qualification plan](QUALIFICATION_PLAN.md) now uses the 0.50 uF floor. Keep E02 open for combined-corner evidence and the actual P02/P03/P04 stability captures.

Keep the supported load at <=100 mA and firmware GCC <=32. At the project's 5.25 V input and 100 mA ceiling, the calculated pass-element load is approximately (5.25-3.3)*0.1 =0.195 W. This is an input for thermal testing, not a measured junction temperature. Keep the existing 10-40 C prototype ambient envelope until P06 passes.

Retain USB-powered operation with SWD target power disabled. Do not externally drive +3V3 while VBUS is absent. Capture the fastest power removal and stalled/off-display case to check output-to-input reverse bias; MCU sleep is not a guaranteed discharge load. The ECO provides no reverse blocking or controlled output-discharge guarantee. EN remains tied to VBUS, so firmware continues controlling the display through U2 SDB rather than U3 enable.

## Implementation and closure record

| Item | Implemented result / evidence |
|---|---|
| Native schematic and library | U3 uses `LDO_TPS7A2433DBVR`; D91 uses `ESD_PESD5Z6_0_UNIDIR_SOD523`, with an updated unidirectional body and K1/A2 pin names. Saved-record parsing confirms both exact MPNs, manufacturer/supplier fields, comments and datasheets in SchDoc and SchLib. |
| Guarded native stage | [Procurement stage report](../native-plan/revc-procurement-stage-report.txt): 135 source components, 29 library components, 346 source pins, 136 library pins; preflight errors 0, final errors 0, protected before/after counts both 4189. `Saved=False` records the stage's intentional no-auto-save behavior; the later persisted-file hashes below establish the saved state. |
| PCB metadata | [Native stage result](../native-plan/revc-pcb-metadata-stage-result.tsv): errors 0. [Saved PCB audit](../../../verification/revc/PCB_METADATA_AUDIT.json): PASS, with preserved physical geometry, nets and rule stream. |
| Assembly specification | [SC-REVC-LCSC-P1 BOM](../assembly/BOM.md) and [placement register](../assembly/PLACEMENT_REGISTER.md) select both replacements. D91's instruction is cathode/pin 1 to VBUS_5V, anode/pin 2 to GND. Delivered package/marking inspection is a first-article test, not completed by a BOM edit. |
| Known positive rating mismatch | **CORRECTED IN CAD AND SELECTED BOM.** This closes the original 10 V clamp versus 6 V regulator-limit defect, within the component-data boundary above. |
| Physical qualification | **NOT TESTED:** startup/load steps, capacitor combined-corner margin, reverse bias, positive/negative system transients, thermal behavior and NFC. P01–P14 in the qualification plan have no physical pass entered. |

Persisted native source binding at this implementation review:

| Native file | SHA-256 |
|---|---|
| `SnakeCard_Native.PcbDoc` | `a3c1bad7ac39c80c9fc90f9bd20d366df778fe45630cac338fb5efb860d3ac84` |
| `SnakeCard_Native.SchDoc` | `e89ae0a8a44abdfa60d52b0149ef2538445f9739bfb2a0a2584648abac8499e6` |
| `SnakeCardNative.SchLib` | `b38650103eb216bfb903f709078879ed2f240235a41dd01132069392601be80b` |

The frozen 43-rule `/Rules6/Data` stream remains `f20b6580b3fa99a5bef3a966ef0c7feff56aca0469f0c2336a1bcbbb12bf7ed7`. Full schematic validation, native DRC and the final CAM/assembly manifest belong to the same-source release evidence; this implementation record is not a substitute for those reports. A later save that changes a source hash must be rebound and checked before release. The [source record](POWER_ECO_SOURCES.json) retains the original component-selection review; its recommendation-only status describes that review, not the subsequent native implementation recorded here.

AP2205-33W5-7 was screened but is not selected because its output-capacitor requirement complicates this otherwise compatible ECO. The applied public Rev C ECO does not change purchased Y4 boards or certify their installed parts.
