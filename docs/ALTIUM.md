# Altium Rev C

Updated 28 October 2026. Open [SnakeCard_RevC.PrjPcb](../altium/SnakeCard_RevC.PrjPcb) in Altium Designer. Rev C is the current prototype design; the root KiCad project is historical.

## Native files

| File | Contents |
|---|---|
| [SnakeCard_Native.SchDoc](../altium/SnakeCard_Native.SchDoc) | Single-sheet schematic grouped by function |
| [SnakeCard_Native.PcbDoc](../altium/SnakeCard_Native.PcbDoc) | Routed Rev C PCB, nominal 0.8 mm stack and repaired R2 outline |
| [SnakeCardNative.SchLib](../altium/SnakeCardNative.SchLib) | 29 local schematic symbols and selected procurement fields |
| [SnakeCardNative.PcbLib](../altium/SnakeCardNative.PcbLib) | 14 electrical footprints and one padless QR artwork model |
| [Schematic PDF](../exports/SnakeCard_RevC_Schematic.pdf) | Native single-sheet export for circuit review |

All 135 schematic instances have local symbol and footprint associations. The source audit checks 346 electrical pins, 39 named nets and 30 isolated no-connect pins. The PCB has 136 component instances, including the board-only QR artwork. Repeated physical pads may share an electrical pin number; U2's grounded exposed pad is electrical pin 41, while its nine paste windows are aperture geometry.

C30 is an unpopulated NFC tuning position. The printed antenna and probe features are not purchased parts. Library geometry is 2D; no complete component 3D-model set is supplied. Current Top/Bottom images are final CAM renders.

## Rev C changes

- One closed 85.6 × 54 mm profile with four R2 corners; duplicate mechanical outline removed and copper repoured.
- Nominal stack saved as 0.72888 mm dielectric plus 0.03556 mm copper per face, excluding solder mask.
- Layout reconstructed under the unchanged 43-rule set, including the required 1.27 mm via lands and 0.7112 mm via holes.
- Controlled procurement variant **SC-REVC-LCSC-P1** applied to schematic, symbol library and PCB metadata.
- U3 changed to **TPS7A2433DBVR** and D91 to polarized **Nexperia PESD5Z6.0,115**; D91 K1 connects to VBUS_5V and A2 to GND.
- REV C / AX-01 artwork and side-correct programming labels: CLK near Top TP7 and DIO near Bottom TP6.
- Final native Gerber, NC and schematic PDF exports reviewed against the saved source.

The [layout tradeoffs](revc/LAYOUT_TRADEOFFS.md) describe uneven fanouts, layer-direction exceptions, the U2 thermal path and preserved orthogonal NFC coil corners. DRC was not relaxed for appearance. The [library override policy](revc/assembly/LIBRARY_OVERRIDE_POLICY.md) controls intentional placed-instance differences; do not apply an unrestricted library update to the board.

## Recorded validation

Native checks used Altium Designer Professional 26.9.1 on the saved engineering source on 28 October 2026.

| Check | Result | Evidence |
|---|---|---|
| PCB DRC after save, close and reopen | 0 violations, 0 warnings, 0 waivers; 31 processed report sections, 43 stored rule records unchanged | [Native DRC](../verification/revc/Final_Reopened_DRC.html) |
| Native schematic validation | 0 errors, 2 reviewed warnings | [Native ERC](../verification/revc/Final_Native_ERC.html) |
| Saved source, bindings and firmware pin contract | Passed | [Source audit](../verification/revc/FINAL_SOURCE_AUDIT.md) |
| Final manufacturing outputs | Nine Gerbers and two NC files checked against source | [CAM review](../verification/revc/CAM_VISUAL_REVIEW.md) |

The two retained compiler warnings are "no driving source" on reset-bias networks:

| Net | Circuit and disposition |
|---|---|
| LED_IICRST | R23 is a 10 kΩ pull-down to GND. U2 pin 39 is an active-high reset input and is held low for normal operation. |
| NRST | R10 is a 10 kΩ pull-up to +3V3, with C12 filtering and TP8 debugger access. |

No No-ERC suppression or pin-type change was used to hide these warnings. Physical reset behavior remains untested.

The public native files have local metadata/path cleanup and container repacking. The PDF's author metadata and reports' workstation paths were removed for publication. The [publication manifest](../verification/revc/PUBLICATION_MANIFEST.json) maps the original verified files to these public copies and records the transformation checks. Electrical content and the 43-rule stream were preserved; a new native DRC run on the repacked public files is not claimed.

## Editing and exporting

1. Open the Rev C project with both local libraries available. Confirm the active board and output directory before exporting on another workstation.
2. Retain controlled MPNs and intentional instance overrides, or record an explicit ECO.
3. After edits, validate the schematic, refill copper, save, close/reopen the PCB and run the full configured DRC.
4. Regenerate the schematic PDF, BOM, placement, drawings, Gerbers and NC files from the same saved revision. Review actual exported geometry and bind new hashes to the checks.
5. Repeat affected physical tests after a circuit, layout, part or firmware change.

The checked Rev C manufacturing files are in [fabrication/revc](../fabrication/revc/). Earlier KiCad exports and Altium records describe their own revisions. Supplier process acceptance and P01–P14 remain open.
