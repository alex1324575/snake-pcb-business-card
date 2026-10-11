# Altium verification records

These records concern the public files in [`altium/`](../../altium/), a separate laboratory variant of the project. The existing KiCad sources and manufacturing exports retain their baseline revision. Read the [variant notes](../../docs/ALTIUM.md) before generating fabrication files.

## Recorded results

| Check | Result | Evidence |
|---|---|---|
| Native project Validate | 0 errors; 2 reviewed reset warnings | [Native validation record](native_validation.json), 2026-10-07 10:37:25, UTC-07:00 |
| Native PCB DRC | 0 violations, 0 warnings, 0 waived violations; 22 rules processed | [Full native DRC report](PCB_DRC.txt), 2026-10-07 10:43:11, UTC-07:00 |
| Public artwork and saved-file comparison | Passed; no privacy findings; both native QR payloads decode to `https://example.com/` | [Saved-file release audit](public_native_audit.json), 2026-10-11, UTC-07:00 |
| Native footprint-library save/reopen confirmation | 195 fills in both QR component group and active workboard; 571 protected records unchanged; 0 errors | [Native validation record](native_validation.json), final read-only probe 2026-10-11 13:14:25, UTC-07:00 |

The native checks ran in Altium Designer Professional 26.9.1 after saving and reopening the public documents. The two schematic warnings remain visible on `LED_IICRST` and `NRST`. Their disposition and the 11 retained development symbol-library findings are documented in [Altium](../../docs/ALTIUM.md). No fresh zero-finding library report is claimed.

The PCB DRC result is bound to the PCB SHA-256 in `native_validation.json`. The report lists the actual constraints and their scopes. Five zero-gap switch rules apply narrowly between each switch's own keepout and its own pads; the global clearance rules remain 0.15 mm and 0.20 mm. A DRC pass covers the configured checks, not all manufacturing constraints.

The [hash manifest](file_hashes.json) identifies the released CAD files and verification artifacts. The [PCB QR raster](PublicNativeBoardQR.png) and [footprint-library QR raster](PublicNativeLibraryQR.png) were generated from the saved native fills for decoding; they are verification images, not fabricated-board photographs.

## Saved-file audit method

The release audit reads the saved native compound-file streams rather than relying on editor screenshots or contact labels. It compares them with a fixed development reference and records the hashes of the five public project files. It checks:

- Exactly four relative project document members: the schematic, PCB and two local libraries.
- The 135 schematic components, 346 electrical pins, primary pin identifiers, coordinates, parameters and footprint-model associations.
- The 29 symbols, 136 library pins and 29 symbol footprint models.
- The PCB's 136 component objects and 366 physical pads, including routing, vias, copper, outline, stackup and component placement.
- The 15 footprint models and 123 library pads. All named streams of the 14 electrical footprint subtrees must match the reference; the remaining footprint is padless QR artwork.
- Public identity and contact text, plus a scan of live streams, directory names and whole-file bytes for private identity, contact and user-path data.
- The actual QR fills in both the PCB and footprint library: 195 rectangles per object, matching primitive-count headers, the verified public coordinates and layers, and a 21 mm extent. Noncoordinate fill flags, ownership and style remain checked. Each saved native geometry is rasterized and decoded to confirm `https://example.com/`.

The public example identity is **Alex Parker**. A contact label or renamed footprint is insufficient: the encoded QR geometry must also pass. Decoding a rasterized native QR does not establish readability of printed silkscreen.

## Native encoding and comparison scope

The comparison allows only documented native save changes: regenerated auxiliary pin-parameter IDs while preserving primary pin associations; editor viewport bounds and host-path/time metadata; exact font-buffer padding normalization; and wide-text index changes that still resolve to the intended text. Text coordinates, layers, visibility and style remain checked.

Five inherited switch-clearance rule layer fields changed from `UNKNOWN` to `TOP` during native saving. Rule identities, scopes, priorities, enabled states and values remain checked. These observations do not justify ignoring arbitrary rule or geometry changes. Container repacking removes unreachable data and unused padding while preserving named-stream content, apart from the documented public text/path/QR changes and exact encoding restoration of the electrical footprint subtrees.

The footprint-library QR replacement changed only its primitive data, primitive-count header and corresponding GUID index data/header. QR parameters and wide-string metadata remained intact, and the 14 electrical footprint subtrees remained exact. The library was then saved and reopened in Altium, and the final saved-file audit passed. The final native reopen probe confirmed the QR object counts without writing CAD files; it was not a new project Validate, PCB DRC or symbol-library rule check.

The resulting records establish preservation within those stated scopes. They do not claim whole-file byte identity. Native embedded and external 3D model streams are empty; the README images are KiCad renders.

## Physical work remains open

No assembled-board power-up, MCU firmware execution, measured power/display performance, USB transient qualification, hidden-joint acceptance or NFC tuning is recorded. All eight [component-review findings](../../docs/COMPONENT_REVIEW.md) remain open. In particular, the retained nearby SW5 ground via and open exposed-pad vias still need a documented assembly process and sample evidence. See [Assembly](../../docs/ASSEMBLY.md) and the [bring-up guide](../../docs/BRINGUP.md).
