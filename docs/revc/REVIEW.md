# Rev C corrective-action record

28 October 2026. Review baseline: engineering review of 26 October 2026.

**Status: CAD corrections, native validation and final CAM verification complete.** This package supports prototype engineering review. Production approval remains on HOLD pending supplier process acceptance and physical qualification; a passing CAD check does not establish an assembled-board result.

The public Rev C directory contains the native project and libraries, [final Gerber/NC archive](../../fabrication/revc), [native schematic PDF](../../exports/SnakeCard_RevC_Schematic.pdf), controlled assembly documents, retained firmware and verification reports. [Final check status](../../verification/revc/FINAL_CHECK_STATUS.json), [native export record](../../verification/revc/FINAL_NATIVE_EXPORT.json) and [file manifest](../../verification/revc/PUBLICATION_MANIFEST.json) control delivery. Historical 27 October CAM is superseded by the verified exports below.

## Verified saved design

- Repaired the native board outline to one 85.6 x 54.0 mm profile with four R2.0 corners; removed duplicate mechanical contour primitives and repoured copper.
- Saved the nominal 0.8 mm stack: 0.72888 mm dielectric plus 0.03556 mm copper on each face, excluding modeled solder mask. Manufactured thickness tolerance and USB connector fit still require acceptance.
- Preserved all **43 design rules**. The final PCB was saved, closed and reopened before [native DRC](../../verification/revc/Final_Reopened_DRC.html) at **28 October 2026, 01:15:55**: **0 violations, 0 warnings, 0 waivers**. The [text report](../../verification/revc/Final_Reopened_DRC.drc) records the checks performed.
- Native project validation at **28 October 2026, 11:16:49** compiled successfully: **0 errors, 2 reviewed warnings**. The [native ERC report](../../verification/revc/Final_Native_ERC.html) and [captured native messages](../../verification/revc/Final_Native_ERC.html) retain both warnings without suppression; their disposition is below.
- Applied U3 **TPS7A2433DBVR** and D91 **Nexperia PESD5Z6.0,115**, with the controlled procurement metadata in SchDoc, SchLib and PCB. D91 is polarized: **K1 to VBUS_5V; A2 to GND**. See the [power ECO record](electrical/POWER_ECO_RECOMMENDATION.md).
- Passed the [saved-source audit](../../verification/revc/FINAL_SOURCE_AUDIT.md): **135 schematic instances, 29 library symbols, 15 footprints, 346 pins, 39 named nets, 30 isolated NC pins and 217 firmware contract checks**. This is semantic, geometry and binding evidence; it is not a bench result or a substitute for native reports.
- Exported all nine final Gerbers at **28 October 2026, 11:12:59** and both native plated-hole/slot NC files at **11:14:50**. [Actual CAM review](../../verification/revc/CAM_VISUAL_REVIEW.md) and [drill audit](../../verification/revc/native_drill_audit.json) pass against the unchanged final saved PCB.
- Exported the [native schematic PDF](../../exports/SnakeCard_RevC_Schematic.pdf) at **28 October 2026, 11:24:51**.

Final PCB SHA-256: `a3c1bad7ac39c80c9fc90f9bd20d366df778fe45630cac338fb5efb860d3ac84`.

Frozen `/Rules6/Data` SHA-256: `f20b6580b3fa99a5bef3a966ef0c7feff56aca0469f0c2336a1bcbbb12bf7ed7`.

The [PCB metadata audit](../../verification/revc/PCB_METADATA_AUDIT.json) verifies pad geometry, via data, component placement/identity, net identity/order and rules. It explicitly accounts for serialized primitive order and component-owner index rebinding; raw file identity is not claimed.

## Native ERC warning review

| Net | Retained native warning | Disposition |
|---|---|---|
| LED_IICRST | No driving source: R23-1, U2-39. | R23 is a 10 kohm pull-down to GND. The driver requires this active-high reset input low during normal operation. Connections and input pin type retained. |
| NRST | No driving source: C12-1, R10-2, TP8-1, U1-6. | R10 is a 10 kohm pull-up to +3V3, with C12 filtering the reset input. Connections and input pin type retained. |

These two compiler warnings remain visible. No No-ERC suppression or rule change was used to produce a zero-warning claim. Reset behavior still requires the physical startup and programming tests.

The IICRST disposition follows the pin-39 description and I2C RESET section of the [Lumissil IS31FL3737B datasheet](https://www.lumissil.com/assets/pdf/core/IS31FL3737B_DS.pdf), Rev. D. The [layout tradeoffs](LAYOUT_TRADEOFFS.md) record the remaining routing and appearance compromises explicitly.

## Assembly and qualification documents

- [Controlled prototype BOM](assembly/BOM.md): SC-REVC-LCSC-P1, 17 procurement groups and 125 fitted parts. The selection does not establish purchase, delivered parts or supplier stock.
- [Placement register](assembly/PLACEMENT_REGISTER.md), [searchable assembly drawings](assembly/ASSEMBLY_REFERENCE.html) and [assembly manual](assembly/ASSEMBLY_MANUAL.md). The process uses Top reflow followed by manual installation of the Bottom MCU/C10 and the five switches. Placement tables use the native Top coordinate frame; Bottom drawings mirror X for a direct underside view.
- [Library artwork controls](assembly/LIBRARY_OVERRIDE_POLICY.md), [fabrication requirements](FABRICATION_NOTES.md), and [electrical/thermal/NFC acceptance record](electrical/QUALIFICATION_PLAN.md).

**All P01-P14 physical tests remain NOT TESTED.** No soldering, voltage, transient, temperature, capacitor combined-corner or NFC result has been entered.

## Finding disposition

| Finding | Completed work | Remaining gate |
|---|---|---|
| M01 - outline | Native shape and mechanical contour corrected and saved; final CAM has one closed eight-segment R2 profile with no invalid arcs. | CAD/CAM correction complete. |
| M02 - stack | Nominal 0.8 mm stack saved and independently read back. | Supplier thickness tolerance and first-article connector fit. |
| M03 - release package | Native DRC/ERC, saved-source audit, final Gerber/NC files, actual CAM review, schematic PDF and hash-controlled review files complete. | Production acceptance remains separate. |
| A01 - procurement | Exact-part metadata applied to native SchDoc/SchLib/PCB; controlled BOM and saved readback complete. | Delivered-part identity, lot/process inspection and first-article qualification. |
| A02 - assembly/programming | Side-correct drawings, polarity instructions, assembly sequence and SWD connections documented. | Execute assembly and programming/fixture checks. |
| E01 - protection | Original positive clamp/LDO rating mismatch corrected in CAD and selected BOM by U3/D91 ECO. | System positive/negative transient and reverse-bias qualification: **NOT TESTED**. |
| E02 - capacitance | C1/C2 acceptance is >=0.50 uF effective after uncertainty and aging allowance; exact-part typical screen recorded. | Combined-corner evidence and startup/load-step tests. |
| E03 - thermal/power | R24 = 20 kohm, GCC <=32 restriction and thermal/supply tests documented. | Temperature, rail stability and supply-transient measurements. |
| R01 - NFC | C30 DNP, tuning method and phone/URL acceptance documented. | Resonance tuning and phone-read trials. |
| Q01 - rule coverage | Final actual CAM copper-edge/paste checks pass; all 195 drill/slot geometries match the final source. | Supplier process acceptance and first-article inspection. |
| L01 - instance graphics | Intended LED/testpoint/legend overrides and update controls documented. | Review future library updates against those controls. |

## CAM status and mixed-source control

The current nine-layer Gerber set was natively exported at **11:12:59 on 28 October 2026**. The [CAM visual review](../../verification/revc/CAM_VISUAL_REVIEW.md) confirms public ALEX XU artwork, **REV C / AX-01**, the final CLK/DIO labels, the four R2 corners and both board faces. The actual Bottom CAM QR decodes to the intended public example URL, `https://example.com/`. File hashes and export times are recorded in [CAM_VISUAL_REVIEW.json](../../verification/revc/CAM_VISUAL_REVIEW.json).

Both native NC files were re-exported at **11:14:50 on 28 October 2026**. The [drill audit](../../verification/revc/native_drill_audit.json) matches **191 plated round holes** (189 vias and two printed-coil holes) and **four horizontal plated USB shell slots, 1.2 x 0.5 mm**, with no NPTH holes: all **195 geometries** match the final saved PCB. Identical NC content hashes after re-export are expected because the hole geometry was unchanged.

Historical Gerbers from **23:07 on 27 October 2026** contain the old REV B legend and are superseded. Do not mix those earlier outputs with the final set. Use export hashes and the delivery manifest rather than filenames alone.

## Packaging and production gates

1. Use the hash-controlled review archive and retain its manifest. Recheck any source or output saved after that binding. No CAD validation or CAM export remains pending.
2. Obtain fabricator acceptance of finished thickness, slot plating/tolerance and stencil/process requirements. No supplier acceptance has been recorded.
3. Assemble a controlled first article and execute P01-P14. Record actual results and resolve failures before claiming the corresponding qualification. All physical tests remain **NOT TESTED**.
