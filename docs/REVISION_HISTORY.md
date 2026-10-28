# Hardware revision history

The repository retains multiple design records. Their files and test evidence are revision-specific.

| Revision / record | Role | Use |
|---|---|---|
| **Altium Rev C — 28 October 2026** | Current prototype engineering design | `altium/SnakeCard_RevC.PrjPcb`, `fabrication/revc/`, `docs/revc/` and `verification/revc/` |
| Earlier Altium native project / October 12 reports | Historical development record | Applies to the hashes recorded by those checks; does not describe the Rev C layout, BOM or SWD access. |
| Root KiCad project and earlier exports | Historical KiCad design | Editable history and its matching reports; not synchronized to Rev C. |
| Firmware v1.0.0 | Retained tested software | Original `RevB` image names retained; native Rev C mapping checked separately. No physical MCU execution recorded. |

## Rev C summary

- Reconstructed layout under the unchanged 43-rule set; final saved-source DRC reported zero violations, warnings and waivers.
- Repaired the 85.6 × 54 mm R2 outline and saved nominal 0.8 mm stack.
- Applied controlled purchasing variant SC-REVC-LCSC-P1, including TPS7A2433DBVR and polarized PESD5Z6.0,115.
- Added final native Gerber/NC and schematic PDF exports, source/CAM/drill checks and controlled assembly records.
- Corrected programming guidance for two-sided access: TP7/SWCLK Top; TP6/SWDIO and other SWD contacts Bottom.
- Retained two reviewed schematic reset-bias warnings without suppression.

Public copies remove workstation metadata; [publication records](../verification/revc/PUBLICATION_MANIFEST.json) map them to the original verified engineering files. Supplier process acceptance and all P01–P14 physical tests remain open.

Use the current [verification](VERIFICATION.md), [assembly](ASSEMBLY.md) and [bring-up](BRINGUP.md) entry points. Do not mix historical manufacturing layers, BOMs, stencil data or contact locations with Rev C.
