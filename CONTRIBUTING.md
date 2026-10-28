# Contributing

This is a prototype hardware and firmware project. Small, well-documented changes are easiest to review.

## Before changing a board

[Altium Rev C](docs/ALTIUM.md) is the current design. The root KiCad project and earlier exports are historical; identify the revision before editing and keep its manufacturing outputs separate. A change to MCU pin mapping, LED polarity, power or driver address must also update firmware assumptions, connection checks and documentation.

For Rev C, run native project validation and full PCB DRC after saving/reopening, then regenerate matching CAM, schematic PDF, BOM, placement and drawings. Preserve the rule set unless a separate engineering change explicitly reviews the rule change; do not relax constraints for appearance. Include source/output hashes, actual exported-geometry review and relevant reports. Repeat affected physical tests after an electrical, layout, component or firmware change.

KiCad changes require matching KiCad ERC/DRC and exports. Their checks do not replace native Rev C checks.

## Firmware changes

From the repository root, run the target build and host tests:

```sh
python firmware/tools/build.py --test-host --sanitize --build
```

The existing checker below reads the **historical KiCad PCB and XML netlist**:

```sh
python firmware/tools/check_board_contract.py
```

It remains useful for that design, but does not regenerate Rev C's native pin-contract audit. The [Rev C verification record](docs/VERIFICATION.md) documents the separate saved-native check. Retained `RevB` image names identify the unchanged tested release, not a claim that later hardware has been exercised.

Add focused tests for gameplay, input, timing or display-transaction changes. State whether a result comes from host testing, inspection or an actual board. Retain the current ceiling and bounded bus timeouts unless a change includes measured evidence and an updated power budget.

## Public-package checks

Run the publication-manifest and local-link check from the repository root:

```sh
python tools/check_revc_release.py
```

This checks public-file integrity and documentation references; it does not run Altium, inspect an assembled board or execute P01–P14. After editing a controlled payload, review the change and update its manifest binding rather than bypassing a failed check.

## Public example data

Use **ALEX XU** and the reserved **https://example.com/** URL for shared artwork and contact assets. Keep screenshots, rendered images, PDF metadata and native-file metadata free of personal contact data, credentials, workstation paths and local caches. Preserve the documented original-to-public file mapping when applying publication-only cleanup.

## Pull requests and issues

Describe the concrete problem, resulting behavior and validation. Hardware reports should include board/firmware revisions, installed BOM variant, supply conditions, measurements and a clear photo or diagram where useful. Firmware failures should include reproducible inputs and expected behavior.

Retain third-party notices when changing or redistributing library or runtime material. A passing CAD or repository check does not establish supplier acceptance, physical qualification or production approval.
