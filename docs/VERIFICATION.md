# Verification and limitations

This release is a **digitally validated prototype**. It has not been assembled, flashed on a board, RF-tuned or tested with phones. Use the reports generated from this public tree; results from another checkout or an earlier artwork revision do not establish the state of these files.

## What each check establishes

| Check | Evidence | Boundary |
|---|---|---|
| Schematic ERC | KiCad report in [verification](../verification/) | Pin-type and electrical-rule consistency |
| PCB DRC and parity | KiCad report in [verification](../verification/) | Design-rule violations, unrouted connections and schematic/PCB correspondence |
| Firmware board contract | [BOARD_CONTRACT_REPORT.md](../firmware/references/BOARD_CONTRACT_REPORT.md) | Actual XML nets, PCB pad nets, all 90 LED positions, firmware dimensions and selected GPIO source contracts |
| Firmware host tests | [Firmware verification](../firmware/verification/) | Game rules, UI/debounce and mocked driver transactions/failure handling |
| Cortex-M0+ build | [Release outputs](../firmware/release/) | Target compilation, link layout, image metadata and hashes |
| QR / NDEF | [Contact validation](../contact/contact_validation.json) and native contact audit in [verification](../verification/) | Decoded URI, native rear artwork orientation and NDEF content |

KiCad checks do not prove every component choice or analog behavior. Host tests do not execute the MCU peripherals. A 3D preview does not certify connector fit, and decoding a rasterized QR does not certify printed contrast.

## Reproduce firmware and mapping checks

From the repository root:

```sh
python -m unittest discover -s firmware/tests -p "test_build*.py"
python firmware/tools/build.py --build
python firmware/tools/build.py --test-host --sanitize
python firmware/tools/check_board_contract.py
```

The build requires an Arm GNU toolchain; host tests need a supported native compiler. See [firmware setup](../firmware/README.md) for tool selection and sanitizer support. The board-contract checker uses Python's standard library and reads the actual root PCB and exported XML. Its report includes input hashes and negative controls that intentionally introduce mapping mistakes.

Artifact checks reject checkout, user-home and private toolchain paths. Public system paths from precompiled runtime libraries (such as `/usr` on Linux) are allowed; compiler prefix maps cannot rewrite those libraries' existing debug records.

For hardware changes, refill zones, save the PCB and rerun ERC/DRC using KiCad. Regenerate the XML netlist before rerunning the board-contract checker. Export fabrication files only after those checks. Review and resolve findings; do not suppress a physical defect merely to obtain an empty report.

After changing contact content, regenerate QR/NDEF and rerun the native contact audit as described in [Customization](CUSTOMIZATION.md). Keep exports, reports and hashes associated with the same source revision.

## Physical work still required

- First-board assembly inspection and SWD programming.
- Supply/current/thermal measurements and LED visibility/ghosting checks.
- Real button response, I²C waveforms, watchdog and recovery tests.
- USB connector fit, hot-plug behavior, ESD and repeated-use testing.
- NFC tuning, actual tag programming and phone read-range testing.
- Scanning the fabricated QR under realistic lighting.

Use [Bring-up](BRINGUP.md) as the sample acceptance procedure. Record measurements and failures before describing a revision as board-tested.
