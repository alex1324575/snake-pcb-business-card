# Historical Altium verification — October 2026

These October 11–12 records apply only to the historical files identified by their recorded hashes. The current files in `altium/` have been replaced by Rev C; these earlier reports do not validate those current files. Use [current verification](../../docs/VERIFICATION.md) and [Rev C evidence](../revc/) for the latest design.

## Results

| Date | Check | Result | Evidence |
|---|---|---|---|
| 2026-10-12 | Project Validate | 0 errors; 2 reviewed reset warnings | [Native validation](native_validation.json) |
| 2026-10-12 | PCB DRC | 0 violations, 0 warnings, 0 waivers; 22 rules processed | [DRC report](PCB_DRC.txt) |
| 2026-10-12 | Saved-file comparison and contact check | Passed; no private identity/path findings; both QR payloads are `https://example.com/` | [Audit](public_native_audit.json) |
| 2026-10-11 | Footprint-library reopen | 195 QR fills in both component group and active workboard; 571 protected records unchanged; 0 errors | [Native validation](native_validation.json) |

Native checks ran in **Altium Designer Professional 26.9.1** after saving and reopening the documents. The two schematic warnings are on `LED_IICRST` and `NRST`. The historical reset dispositions were a pull-down on LED_IICRST and a pull-up/filter on NRST. The historical symbol-library check retained **11 findings**: ten missing pin-name fields in passive/ESD symbols and one noncontinuous pin-number sequence in the power-only USB-C symbol. These are historical audit records, not a current Rev C library-check result. The library reopen was a read-only geometry check, not a repeat of project Validate, PCB DRC or the symbol-library rule check.

The PCB DRC result is bound to the SHA-256 in `native_validation.json`. Five zero-gap switch rules apply between each switch's own keepout and pads; global clearances remain **0.15 mm** and **0.20 mm**. The report lists rule values and scopes. The [hash manifest](file_hashes.json) identifies the CAD and verification files.

## Saved-file comparison

The audit reads native compound-file streams and compares them with a fixed development reference. It checks:

- Four relative project documents: schematic, PCB and both local libraries.
- **135 schematic components and 346 electrical pins:** primary pin identifiers, coordinates, parameters and footprint bindings.
- **29 symbols, 136 library pins and 29 symbol footprint models.**
- **136 PCB component objects and 366 pads:** placement, routing, vias, copper, outline and stackup.
- **15 footprint models and 123 library pads:** all named streams of the 14 electrical footprint subtrees match the reference; the remaining model is padless QR artwork.
- Public contact text and absence of private identity, contact and user-path data in live streams, directory names and whole-file bytes.
- Both QR objects: **195 native fills**, matching count headers, verified coordinates/layers, a **21 mm** extent, and unchanged noncoordinate flags, ownership and style.

The public identity is **Alex Xu**. The [PCB QR raster](PublicNativeBoardQR.png) and [library QR raster](PublicNativeLibraryQR.png) were generated from saved native fills and decode to `https://example.com/`. They are verification images; printed silkscreen readability still needs a physical check.

## Native encoding changes

The audit distinguishes design changes from native serialization changes. Allowed differences are recorded in the audit:

- Regenerated auxiliary pin-parameter IDs with primary pin associations preserved.
- Editor viewport bounds and host-path/time metadata.
- Exact font-buffer padding normalization and wide-text index changes that resolve to the intended text. Text position, layer, visibility and style remain checked.
- Five inherited switch-clearance layer fields changed from `UNKNOWN` to `TOP`. Rule identity, scope, priority, enabled state and value remain checked.

Container repacking removes unreachable data and unused padding. Named-stream content is preserved except for documented public text/path/QR changes and exact encoding restoration of the electrical footprint subtrees. The library QR replacement changes its primitive data, count header and corresponding GUID index data/header; QR parameters and wide-string metadata remain intact. These are scoped preservation checks, not whole-file byte identity.

Native embedded and external 3D model streams were empty. The historical renders came from KiCad; the current Rev C Top/Bottom images are CAM renders.

## Outstanding hardware validation

At this historical review, eight component/process findings remained open, including the nearby SW5 ground via and exposed-pad via process. Those findings describe the recorded layout, not the reconstructed Rev C geometry. No physical pass was recorded. The current [component register](../../docs/COMPONENT_REVIEW.md), [assembly guide](../../docs/ASSEMBLY.md) and [P01–P14 record](../../docs/revc/electrical/QUALIFICATION_PLAN.md) supersede the build guidance; all 14 current physical tests remain NOT TESTED.
