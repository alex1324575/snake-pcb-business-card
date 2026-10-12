# Altium project

Open [SnakeCard_Native.PrjPcb](../altium/SnakeCard_Native.PrjPcb) in Altium Designer. It contains a single-sheet schematic, a routed PCB, and local symbol and footprint libraries.

## Files and libraries

| File | Contents |
|---|---|
| [SnakeCard_Native.SchDoc](../altium/SnakeCard_Native.SchDoc) | Schematic organized by function on a 10 mil grid |
| [SnakeCard_Native.PcbDoc](../altium/SnakeCard_Native.PcbDoc) | Laboratory layout revision |
| [SnakeCardNative.SchLib](../altium/SnakeCardNative.SchLib) | 29 symbols, including value and assembly variants |
| [SnakeCardNative.PcbLib](../altium/SnakeCardNative.PcbLib) | 14 electrical footprints and one padless QR artwork model |

All 135 schematic components have local library and footprint associations. The schematic has 346 electrical pins; the symbol library has 136 pins across its 29 symbols. Repeated physical pads share their electrical pin numbers.

U2 has 41 electrical pins, including the grounded exposed pad. Its nine paste windows are aperture geometry, not additional pins. C30 is a separate unpopulated NFC tuning position. The printed antenna and QR artwork are excluded from the purchasing BOM.

The footprint library contains 2D geometry without 3D component models. The repository's board images are KiCad renders.

## Differences from the KiCad layout

The Altium project derives from a later laboratory revision. It includes:

- Relocation of four button-signal vias.
- Corrections to imported pad shapes, mask and paste geometry.
- Filled QR artwork and restored switch keepouts.
- Selected silkscreen clearance corrections.
- No paste on C30 or SW1-SW5; switches are hand-soldered after reflow and cleaning.

A ground via remains near SW5. This deviation is still open under **ASM-01** in the [component review register](COMPONENT_REVIEW.md).

The proposed stencil is top-only, frameless, and 0.10 mm thick. U2 retains nine exposed-pad windows. See [Assembly](ASSEMBLY.md) for aperture dimensions and the switch-last sequence; the reflow process still needs sample validation.

Generate a matching BOM, placement file, drawings, Gerbers, and drills from Altium when building this revision. The existing `exports/` and `fabrication/` directories belong to the KiCad revision.

## Validation

| Check | Result | Date |
|---|---|---|
| Project Validate | 0 errors, 2 reviewed warnings | 2026-10-12 |
| PCB DRC | 0 violations, 0 warnings, 0 waivers; 22 rules processed | 2026-10-12 |
| Saved-file and contact audit | Passed; both QR objects decode to `https://example.com/` | 2026-10-12 |
| Footprint-library reopen check | 15 models, 123 pads; 195 QR fills in both model and workboard; 0 errors | 2026-10-11 |

Native validation and the library reopen check used Altium Designer Professional 26.9.1. The [native validation record](../verification/altium/native_validation.json), [DRC report](../verification/altium/PCB_DRC.txt), and [saved-file audit](../verification/altium/public_native_audit.json) identify the checked files. Comparison methods and file hashes are in [Altium verification](../verification/altium/README.md).

The schematic warnings are on `LED_IICRST` and `NRST`, both passive reset networks. `LED_IICRST` has an intentional pull-down; `NRST` has a pull-up, filter capacitor, and debugger test point.

The development symbol-library check retained 11 findings: ten missing pin-name fields in passive/ESD symbols and one noncontinuous pin-number sequence in the power-only USB-C symbol. Pin numbers and footprint mappings are present. This library check predates the public artwork update.

Assembly, MCU execution, USB transient and thermal measurements, and NFC tuning remain pending. Use the [bring-up guide](BRINGUP.md) for the first assembled board.
