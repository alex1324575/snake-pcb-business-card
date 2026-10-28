# Rev C fabrication acceptance specification

28 October 2026. **Prototype engineering review - production approval remains on HOLD.** Native outline/stack corrections, native validation and final same-source Gerber/NC verification are complete. Final reopened DRC reports 0 violations, 0 warnings and 0 waivers. Native schematic validation reports 0 errors and 2 reviewed passive-network warnings, retained without suppression. Supplier process acceptance and all physical qualification remain open.

Saved PCB SHA-256: `a3c1bad7ac39c80c9fc90f9bd20d366df778fe45630cac338fb5efb860d3ac84`. See [saved-source audit](../../verification/revc/FINAL_SOURCE_AUDIT.md), [native DRC](../../verification/revc/Final_Reopened_DRC.html), [native ERC](../../verification/revc/Final_Native_ERC.html), [actual CAM review](../../verification/revc/CAM_VISUAL_REVIEW.md), [native schematic PDF](../../exports/SnakeCard_RevC_Schematic.pdf) and [package status](REVIEW.md).

## Fabrication requirements

| Requirement | Target / acceptance |
|---|---|
| Finished profile | 85.6 x 54.0 mm, four R2.0 corners; one closed, unambiguous profile matching the saved native outline |
| Coordinates | Native X100.0-185.6 mm, Y0.0-54.0 mm; common CAM/drill origin; millimeters; no rescaling |
| Material / layers | FR-4, two copper layers |
| Thickness | Nominal 0.8 mm for UJC-HP-3-SMT-TR; supplier finished tolerance and first-article connector fit require acceptance |
| Saved CAD stack | 0.72888 mm dielectric + 0.03556 mm copper each face = 0.80000 mm excluding modeled mask; modeled mask is 0.01016 mm per face |
| Copper | 1 oz nominal outer layers; no controlled-impedance interconnect is specified |
| Finish / appearance | ENIG, black solder mask, white silkscreen; public ALEX XU artwork and REV C / AX-01 legend |
| Round plated holes | 191 total: 189 via holes diameter 0.7112 mm and two printed-coil holes diameter 0.3 mm; final NC positions/tool sizes independently matched to source |
| Via lands | Diameter 1.27 mm; both faces open under the preserved testpoint/via rules; no tenting substitution |
| Plated slots | Four J1 shell slots, 1.2 x 0.5 mm overall; preserve horizontal orientation and plating; supplier finished tolerances and connector fit require acceptance |
| NPTH | None in the current source; do not add unverified drill data |
| Stencil | Top only for Top-reflow / Bottom-manual assembly; 0.10 mm is the assessed thickness; supplier printing/process acceptance remains required |
| Electrical test | Fabricator netlist test; source/CAM consistency and bare-board continuity before assembly |

## Independent geometry screens

The final same-source CAM screen measures minimum copper-to-edge distances of **0.3822 mm Top** and **0.33305 mm Bottom**, with no copper outside the repaired R2 envelope. Its evaluated paste contains 294 Top and 34 Bottom flashes. At 0.10 mm stencil thickness, minimum calculated aperture area ratio is **0.76419 Top / 1.94585 Bottom**, and minimum web is **0.14998 mm Top / 0.30000 mm Bottom**. Aperture calculations join the primitive pieces belonging to each flash and use 0.0002 mm arc approximation.

These final-export geometric checks pass; they do not establish measured printing/reflow yield. The documented process does not print Bottom paste. U2 has nine separate exposed-pad paste windows; retain those windows rather than one solid opening. The [drill audit](../../verification/revc/native_drill_audit.json) matches all 191 round holes and four 1.2 x 0.5 mm plated slots to the final saved PCB; no NPTH feature exists.

The saved rules remain unchanged, including 0 mm global edge clearance and 0 mm silk-to-silk clearance. Native DRC alone does not enforce every fabrication requirement. Use independent CAM review and supplier acceptance in addition to the [final DRC report](../../verification/revc/Final_Reopened_DRC.html). [JLCPCB capabilities](https://jlcpcb.com/capabilities/pcb-capabilities/) were consulted on 27 October 2026; no acceptance of this quoted thickness, slots or stencil process has been recorded.

## Package controls

All nine current Gerbers were exported natively at **28 October 2026, 11:12:59**; both NC files were re-exported at **11:14:50**. [Actual CAM review](../../verification/revc/CAM_VISUAL_REVIEW.md) confirms the final REV C / AX-01 and SWD legends, profile and both board faces. [CAM_VISUAL_REVIEW.json](../../verification/revc/CAM_VISUAL_REVIEW.json) records the source/output hashes and export times. The native schematic PDF was exported at **11:24:51**.

The historical **27 October 2026, 23:07** Gerbers contain the old REV B legend and are superseded. Do not combine those earlier files with the verified final set.

The final package must contain:

- Matching Top/Bottom copper, mask, silk and relevant paste; one closed profile; plated round-hole and slot data. No NPTH file is required when no NPTH feature exists.
- Fabrication notes/drawing, controlled BOM, native placement with explicit side/angle convention, and assembly drawings.
- Firmware binary/hash, final native DRC/ERC reports, native schematic PDF, independent source/CAM/drill checks and a complete source/output hash manifest.

Final CAM inspection has confirmed the outline, slot sizes/orientation, mask/paste geometry, REV C / AX-01 legend and final SWD labels. Archive/manifest packaging must retain this exact source/output binding; a missing item or mixed revision invalidates that package. Production approval still requires supplier acceptance and physical qualification. After fabrication, follow the [assembly manual](assembly/ASSEMBLY_MANUAL.md) and [qualification record](electrical/QUALIFICATION_PLAN.md); all P01-P14 physical tests remain **NOT TESTED**.
