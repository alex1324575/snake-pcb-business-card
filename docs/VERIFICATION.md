# Verification and limitations

Updated 2026-10-11.

The KiCad baseline is a **digitally validated prototype**. A separate [Altium laboratory variant](ALTIUM.md) is included. Neither has a recorded assembled-board power-up, MCU firmware test, NFC tuning result or phone test. Use reports bound to the specific public files; results from another checkout or an earlier artwork revision do not establish the state of those files.

## What each check establishes

| Check | Evidence | Boundary |
|---|---|---|
| Schematic ERC | KiCad report in [verification](../verification/) | Pin-type and electrical-rule consistency |
| PCB DRC and parity | KiCad report in [verification](../verification/) | Design-rule violations, unrouted connections and schematic/PCB correspondence |
| Firmware board contract | [BOARD_CONTRACT_REPORT.md](../firmware/references/BOARD_CONTRACT_REPORT.md) | Actual XML nets, PCB pad nets, all 90 LED positions, firmware dimensions and selected GPIO source contracts |
| Firmware host tests | [Firmware verification](../firmware/verification/) | Game rules, UI/debounce and mocked driver transactions/failure handling |
| Cortex-M0+ build | [Release outputs](../firmware/release/) | Target compilation, link layout, image metadata and hashes |
| QR / NDEF | [Contact validation](../contact/contact_validation.json) and native contact audit in [verification](../verification/) | Decoded URI, native rear artwork orientation and NDEF content |
| Procurement | [Component review](COMPONENT_REVIEW.md) and [per-reference table](../verification/hardware/component_review.csv) | Exact MPN coverage and native/export consistency; physical and assembly findings remain open |
| Altium project Validate | [Native validation record](../verification/altium/native_validation.json) | Saved/reopened public files: 0 errors and 2 reviewed reset warnings on 2026-10-07 |
| Altium PCB DRC | [Full native DRC report](../verification/altium/PCB_DRC.txt) | Public PCB: 0 violations, warnings or waivers; 22 rules processed; PCB hash recorded in the validation record |
| Altium artwork and saved-file audit | [Saved-file audit](../verification/altium/public_native_audit.json) and [hash manifest](../verification/altium/file_hashes.json) | Passed on 2026-10-11; native geometry/stream preservation, public identity and both decoded QR payloads |

KiCad checks do not prove every component choice or analog behavior. Host tests do not execute the MCU peripherals. A 3D preview does not certify connector fit, and decoding a rasterized QR does not certify printed contrast.

The procurement metadata audit compares the procurement update with the earlier public revision and confirms that electrical connections and PCB geometry were preserved. A separate schematic annotation audit records two corrected notes: the documented LED current now matches the existing firmware's GCC=32. The older public-identity equivalence report remains historical evidence for its recorded board hash; the metadata audit links that hash to the baseline board. The KiCad ERC/DRC, contact and board-contract reports were regenerated for the September 27 baseline update. Adding the Altium variant does not regenerate those artifacts or make them Altium validation evidence.

## Altium result scope

The public files were saved, reopened and checked in Altium Designer Professional 26.9.1 on **2026-10-07, UTC-07:00**. Project Validate at **10:37:25** reported **0 errors and 2 reviewed warnings** on `LED_IICRST` and `NRST`. PCB DRC at **10:43:11** processed **22 rules** and reported **0 violations, 0 warnings and 0 waived violations**. The [validation record](../verification/altium/native_validation.json) includes the PCB SHA-256, and that PCB remained unchanged after the run. The [full DRC report](../verification/altium/PCB_DRC.txt) lists the actual constraints and scopes.

The earlier development Validate/DRC results from October 5/3 remain historical evidence for their source revision. The development symbol-library check retained **11 naming/sequence findings**; no fresh zero-finding library result is claimed.

The public artwork/privacy and saved-file release audit passed on **2026-10-11, UTC-07:00**. Both the PCB and footprint-library QR have 195 native fills, consistent primitive-count headers and a 21 mm extent; their actual saved geometry decodes to `https://example.com/`. The footprint library was saved and reopened in Altium. A final read-only native probe at **13:14:25** found 195 QR fills in both the component group and active workboard, 571 protected records unchanged, and 0 errors. This probe did not write CAD files. See the [audit methodology and encoding scope](../verification/altium/README.md), [native validation record](../verification/altium/native_validation.json) and [published file hashes](../verification/altium/file_hashes.json).

Saved-file comparisons establish pin, net, geometry, parameter and footprint-model preservation within their stated scope. Altium's auxiliary ID and editing/index serialization changes are recorded separately; a semantic preservation result does not assert whole-file byte identity. A native DRC pass covers its configured rules and source revision. Neither check establishes solderability, hidden-joint quality, electrical transients, thermal behavior or NFC performance.

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

A five-board laboratory fabrication batch has been ordered. Receipt, stencil/component purchases and assembly are unconfirmed; no physical test result is recorded. All [component review](COMPONENT_REVIEW.md) findings remain open.

- First-board assembly inspection and SWD programming.
- Supply/current/thermal measurements and LED visibility/ghosting checks.
- Real button response, I²C waveforms, watchdog and recovery tests.
- USB connector fit, hot-plug behavior, ESD and repeated-use testing.
- NFC tuning, actual tag programming and phone read-range testing.
- Scanning the fabricated QR under realistic lighting.

Use [Bring-up](BRINGUP.md) as the sample acceptance procedure. Record measurements and failures before describing a revision as board-tested.
