# Contributing

This is a prototype hardware and firmware project. Small, well-documented changes are easiest to review.

## Before changing a board

Choose the KiCad or [Altium](docs/ALTIUM.md) revision before editing; their layouts and manufacturing outputs differ. A change to the MCU pin map, LED polarity, power circuit, or driver address must also update the firmware, mapping checks, and documentation.

Run KiCad ERC/DRC for KiCad changes, or project Validate and PCB DRC for Altium changes. Regenerate matching manufacturing files and images from the edited project. Include the relevant reports and explain any remaining violations.

## Firmware changes

Run the target build and portable host tests:

```sh
cd firmware
python tools/build.py --test-host --sanitize --build
python tools/check_board_contract.py
```

Add focused tests for changes to gameplay, input handling, timing, or display transactions. State whether a result comes from a host test, static inspection, or an actual board. Keep the display's conservative current limit and bounded bus timeouts unless a change includes measured evidence and an updated power budget.

## Public example data

Keep shared screenshots, PCB artwork, QR codes, and NFC files on a fictional identity and a reserved example URL. Do not submit personal contact information, access tokens, workstation paths, local build caches, or photographs that unintentionally expose someone's details.

## Pull requests and issues

Describe the problem, the resulting behavior, and the validation performed. For hardware reports, include board revision, firmware revision, supply conditions, relevant measurements, and a clear photo or diagram if available. For firmware failures, include reproducible inputs and expected behavior.

The repository contains third-party library and runtime material with its own notices; retain those notices when changing or redistributing those components.
