# Verification

Updated 28 October 2026. This page summarizes the current **Altium Rev C** evidence. CAD and document review is complete; supplier process acceptance is pending and **P01–P14 are all NOT TESTED**. No assembled-board power-up, environmental result or production qualification is recorded.

## Current Rev C results

| Check | Result | Evidence |
|---|---|---|
| Native PCB DRC | 0 violations, 0 warnings, 0 waivers; saved, closed and reopened before the run | [Native report](../verification/revc/Final_Reopened_DRC.html) |
| Rule preservation | All 43 stored native rule records unchanged; 31 sections processed in the final DRC report | [Final status](../verification/revc/FINAL_CHECK_STATUS.json) |
| Native schematic validation | Compile successful, 0 errors and 2 reviewed warnings | [Native report](../verification/revc/Final_Native_ERC.html), [warning review](../verification/revc/NATIVE_VALIDATION_REVIEW.md) |
| Saved schematic/library/PCB audit | 135 schematic instances, 29 symbols, 15 footprints, 346 pins, 39 named nets, 30 isolated NC pins; 217 firmware contract checks passed | [Source audit](../verification/revc/FINAL_SOURCE_AUDIT.md) |
| PCB metadata and geometry | Controlled MPNs/comments applied; physical placement, pad/via data, nets and rules preserved under the documented comparison method | [PCB audit](../verification/revc/PCB_METADATA_AUDIT.json) |
| Final CAM | Nine native Gerbers checked for profile, copper edge, mask, paste, artwork and revision | [Actual CAM review](../verification/revc/CAM_VISUAL_REVIEW.md) |
| Native NC geometry | 191 plated round holes and four 1.2 × 0.5 mm plated slots match the saved PCB; no NPTH | [Drill audit](../verification/revc/native_drill_audit.json) |
| Native schematic PDF | Single-sheet export and rendered page/detail visually reviewed | [PDF](../exports/SnakeCard_RevC_Schematic.pdf), [review](../verification/revc/SCHEMATIC_PDF_REVIEW.json) |
| Controlled assembly package | 125 fitted parts: 123 Top, 2 Bottom; 17 purchasing groups; four drawings reviewed | [Assembly validation](revc/assembly/assembly_validation.json) |

The [final status](../verification/revc/FINAL_CHECK_STATUS.json) and [native export record](../verification/revc/FINAL_NATIVE_EXPORT.json) retain the engineering source/output binding. The PCB DRC ran on 28 October 2026 at 01:15:55 local time. Gerbers were exported at 11:12:59, NC files at 11:14:50, schematic validation ran at 11:16:49 and the schematic PDF was exported at 11:24:51.

## Public copies and recorded hashes

The reports describe the original verified engineering files. Public CAD copies have local metadata/path cleanup and container repacking; publication also removes PDF author metadata and workstation paths from report copies. The [publication manifest](../verification/revc/PUBLICATION_MANIFEST.json) records both original and published hashes, transformations and preservation checks. Electrical data and the frozen 43-rule stream remain unchanged. A fresh native DRC/ERC run on the repacked public copies is not claimed.

## Retained schematic warnings

Both warnings concern intentional reset-bias networks and remain visible in the native report:

- **LED_IICRST:** no driving source on R23-1 / U2-39. R23 is a 10 kΩ pull-down to GND; the driver requires this active-high reset input low during normal operation.
- **NRST:** no driving source on C12-1 / R10-2 / TP8-1 / U1-6. R10 is a 10 kΩ pull-up to +3V3, with C12 filtering the MCU reset input.

No No-ERC suppression, pin-type alteration or rule relaxation was used to claim zero warnings. Startup and programming recovery still require bench tests.

## Evidence boundaries

Native DRC checks configured constraints. Independent final-CAM review supplements gaps in the frozen rules, which include zero global board-edge and silk-to-silk clearances. The measured minimum copper-to-edge distances are **0.3822 mm Top / 0.33305 mm Bottom**. Paste aperture calculations and drill/source comparisons are geometric screens; they do not qualify a supplier's process.

The saved-source audit compares native circuit semantics, footprint bindings and documented geometry invariants. Native serialization order and component-owner indexes are handled explicitly. It does not claim byte identity of files saved by Altium, nor replace native ERC/DRC.

Firmware host tests cover game logic, application behavior and mocked display transactions/failure handling. The retained target build checks compilation and linking. The 217 native Rev C contract checks establish selected connection/mapping agreement; they do not execute MCU peripherals or prove hardware timing. Original `RevB` image names remain because the tested firmware source was not changed for this CAD revision.

## Reproduce software checks

From the repository root:

```sh
python -m unittest discover -s firmware/tests -p "test_build*.py"
python firmware/tools/build.py --build
python firmware/tools/build.py --test-host --sanitize
```

See [firmware setup](../firmware/README.md) for toolchains and sanitizer support. The existing `firmware/tools/check_board_contract.py`, procurement/export tools and root KiCad reports target the historical KiCad inputs. Running them does not regenerate the native Rev C audit.

For native CAD changes, follow [Altium](ALTIUM.md), regenerate affected outputs from the same saved revision and record new hashes. Keep BOM, placement, stencil, manufacturing layers, firmware identity and reports together. A filename alone does not identify a checked revision.

## Physical qualification and acceptance

The [qualification record](revc/electrical/QUALIFICATION_PLAN.md) defines P01–P14, measurements and required evidence. Every result is currently **NOT TESTED**. It includes inspection, startup/shutdown, load response, I²C timing, thermal/current testing, pixels/buttons, fault recovery, SWD repeatability, hot-plug, NFC, positive/negative transients and regulator reverse bias.

Supplier acceptance is required for finished board thickness, USB plated-slot tolerances, stencil printing and assembly inspection. Effective capacitance at C1/C2, U2 heat flow, U3 reverse stress, system protection and NFC tuning require physical evidence. [Layout tradeoffs](revc/LAYOUT_TRADEOFFS.md) and [component review](COMPONENT_REVIEW.md) preserve these limits.

Historical KiCad and earlier Altium reports remain for traceability and apply only to their recorded inputs. They do not upgrade earlier boards to Rev C or establish a physical pass.
