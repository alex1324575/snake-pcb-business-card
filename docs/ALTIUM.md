# Altium laboratory variant

Updated 2026-10-11. Open [SnakeCard_Native.PrjPcb](../altium/SnakeCard_Native.PrjPcb) in Altium Designer. The project has a labeled single-sheet schematic, routed PCB and local libraries. No scripts are required to open or edit it.

## Project documents

| File | Purpose |
|---|---|
| [SnakeCard_Native.SchDoc](../altium/SnakeCard_Native.SchDoc) | Single-sheet schematic with functional section headings |
| [SnakeCard_Native.PcbDoc](../altium/SnakeCard_Native.PcbDoc) | Later laboratory layout with reviewed native import corrections |
| [SnakeCardNative.SchLib](../altium/SnakeCardNative.SchLib) | 29 descriptive symbols with parameters and footprint models |
| [SnakeCardNative.PcbLib](../altium/SnakeCardNative.PcbLib) | 15 models: 14 electrical footprints and one padless QR artwork model |

The development project's saved library associations cover all 135 schematic components and 346 electrical pins. Its 29 symbols preserve value and assembly variants, including C30's separate DNP/no-paste tuning position. Repeated physical pads retain their shared electrical numbers. U2 has 41 electrical pins including its grounded exposed pad; its nine paste-only windows add no electrical pins. The PCB's additional QR object is graphical, and the printed NFC coil is excluded from the purchasing BOM.

The footprint library contains native 2D geometry without embedded 3D component models. The README images remain KiCad renders of the public baseline, using Alex Parker and `https://example.com/`. They are not Altium renders or photographs of an assembled board.

## Variant scope

The root KiCad sources and existing `exports/` and `fabrication/` files retain their baseline revision. The Altium project derives from a later laboratory layout. Changes include relocating four button-signal vias, restoring native paste/mask and pad-shape representations, restoring QR fills and switch keepouts, and correcting selected silkscreen clearances. The schematic uses a 10 mil grid.

The button-via relocation mitigates the earlier under-switch finding, but a ground via remains near SW5. This is an open process deviation without manufacturer approval. Tenting, a passing DRC or hand soldering does not close ASM-01. See [Component review](COMPONENT_REVIEW.md).

The laboratory paste treatment excludes C30 and SW1-SW5. Reflow the other components, complete cleaning and drying, then hand-solder the switches last. The proposed stencil is top-only, frameless and 0.10 mm thick, with the nine U2 exposed-pad windows retained. This is a calculated starting process, not a qualified reflow recipe. See [Assembly](ASSEMBLY.md).

Choose one source variant and regenerate its matching BOM, drawings, Gerbers, drills and validation records before fabrication. The existing KiCad fabrication package is not an export of this Altium layout. The previously ordered laboratory batch does not establish manufacturing approval for the public artwork.

## Development validation record

These results describe the saved development files before the public artwork change. They are historical evidence for that source revision, not fresh native checks of the published example.

| Check | Recorded result | Date / scope |
|---|---|---|
| Saved local library associations | 135/135 components; 29 symbols / 136 library pins; 15 footprint models | Saved/reopened development documents |
| Schematic graph | 39 named nets and 30 isolated no-connect groups; no shorts or pin-net mismatches | Independent saved-file comparison |
| Altium project Validate | 0 errors, 2 reviewed warnings | 2026-10-05, after save/reopen |
| Full native PCB DRC | 0 violations, 0 warnings, 0 waived violations | 2026-10-03; development PCB unchanged afterward |
| Native library Component Rule Check | 11 reviewed findings retained | All eight check options enabled |
| Pad and paste review | 489 PCB/library pads checked; 328 top apertures and zero bottom apertures | Development manufacturing-output comparison |

The two schematic warnings are no-driving-source findings on `LED_IICRST` and `NRST`. The former uses an intentional pull-down for the driver's active-high I2C reset; the latter uses a passive reset pull-up/filter and debugger test point. Their disposition is an engineering review, and both warnings remain visible.

The library check retains ten missing-pin-name findings in four capacitor symbols, five resistor symbols and one ESD symbol. The remaining finding concerns the power-only USB-C connector's deliberately noncontinuous contact numbering. Required pin numbers and footprint mappings are present. This is not a zero-finding library report.

Altium saving regenerated auxiliary pin-parameter IDs and changed scoped editing-access/index metadata. Independent comparison preserved the actual source pin records and all library electrical, visibility, coordinate, parameter and model fields. The separate provenance-aware check passed; the original strict raw-byte gate remained a recorded failure. No whole-library byte-identity claim is made.

## Published example verification

| Check on the public Altium files | Current status |
|---|---|
| Public artwork and saved-file audit | Passed; both saved QR objects have 195 fills and decode to `https://example.com/`; no privacy findings; 2026-10-11 |
| Native project Validate after public artwork change | 0 errors, 2 reviewed reset warnings; 2026-10-07 10:37:25, UTC-07:00 |
| Native PCB DRC after public artwork change | 0 violations, 0 warnings, 0 waived violations; 22 rules processed; 2026-10-07 10:43:11, UTC-07:00 |
| Native footprint-library save/reopen confirmation | 195 QR fills in both component group and active workboard; 571 protected records unchanged; 0 errors; final read-only probe 2026-10-11 13:14:25, UTC-07:00 |

The fresh checks ran in Altium Designer Professional 26.9.1 on the saved and reopened public files. The schematic retained the same `LED_IICRST` and `NRST` warnings described above. See the [native validation record](../verification/altium/native_validation.json) and [full PCB DRC report](../verification/altium/PCB_DRC.txt). The DRC record binds the result to the PCB SHA-256; that PCB was unchanged after the run.

The [saved-file release audit](../verification/altium/public_native_audit.json) passed on 2026-10-11 after the footprint library was saved and reopened in Altium. It checks the actual native QR geometry, preserved circuit and footprint data, and public identity/contact content. The [verification notes](../verification/altium/README.md) explain the comparison scope and permitted native encoding differences; the [hash manifest](../verification/altium/file_hashes.json) identifies the published files. Public artwork changes create a new file revision even when they preserve the circuit.

No physical assembly, firmware execution on the MCU, USB transient/thermal qualification or NFC tuning has been demonstrated. The [verification record](VERIFICATION.md) and [bring-up guide](BRINGUP.md) define the remaining work.
