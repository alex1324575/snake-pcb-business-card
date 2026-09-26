# Contributing

This is a prototype hardware and firmware project. Small, well-documented changes are easiest to review.

## Before changing a board

Open the native KiCad project and preserve the current routing. A change to the MCU pin map, LED polarity, power circuit, or driver address must also update the firmware, board-contract checks, and documentation.

Run KiCad ERC and DRC after schematic or PCB changes. Regenerate affected manufacturing files and images from the edited native project. Include the relevant report and describe any remaining violations rather than suppressing them to obtain a clean count.

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
