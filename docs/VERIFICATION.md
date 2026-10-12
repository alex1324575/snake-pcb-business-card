# Verification

Updated 2026-10-11.

Recorded checks cover the KiCad baseline and the separate [Altium laboratory variant](ALTIUM.md). No assembled-board power-up or measured hardware result is recorded. The two variants have separate source files and reports; the KiCad fabrication exports retain their existing revision.

## Results and evidence

| Check | Evidence | Scope |
|---|---|---|
| KiCad schematic ERC | [Verification reports](../verification/) | Electrical rule consistency |
| KiCad PCB DRC and parity | [Verification reports](../verification/) | Design rules, unrouted connections and schematic/PCB correspondence |
| Firmware board contract | [Board contract report](../firmware/references/BOARD_CONTRACT_REPORT.md) | XML and PCB pad nets, all 90 LED positions, display dimensions and selected GPIO assignments |
| Firmware host tests | [Firmware verification](../firmware/verification/) | Game logic, UI/debounce and mocked driver transactions and failure handling |
| Cortex-M0+ build | [Release outputs](../firmware/release/) | Target compilation, link layout, image metadata and hashes |
| QR and NDEF | [Contact validation](../contact/contact_validation.json) and [native contact audit](../verification/) | Decoded URI, rear artwork orientation and NDEF content |
| Procurement | [Component review](COMPONENT_REVIEW.md) and [per-reference table](../verification/hardware/component_review.csv) | MPN coverage and native/export consistency |
| Altium project Validate | [Native validation record](../verification/altium/native_validation.json) | 2026-10-07: 0 errors; 2 reviewed reset warnings |
| Altium PCB DRC | [Native DRC report](../verification/altium/PCB_DRC.txt) | 2026-10-07: 0 violations, 0 warnings, 0 waivers; 22 rules processed |
| Altium saved-file comparison and contact check | [Audit](../verification/altium/public_native_audit.json), [file hashes](../verification/altium/file_hashes.json) | 2026-10-11: preservation checks passed; both saved QR objects decode to `https://example.com/` |

## Revision and comparison scope

The KiCad ERC/DRC, contact and board-contract reports were regenerated for the September 27 baseline update. The procurement metadata audit confirms that the update preserved electrical connections and PCB geometry. The schematic annotation audit records two corrected notes so the documented LED current matches the firmware's GCC limit of 32. The earlier public-identity equivalence report applies to its recorded board hash; the metadata audit links it to the current baseline.

The Altium checks ran on saved and reopened public files in **Altium Designer Professional 26.9.1**. The two project warnings are "no driving source" on `LED_IICRST` and `NRST`; their disposition is in [Altium](ALTIUM.md). The PCB hash in the validation record identifies the board used for DRC. Earlier October 3/5 development checks apply to their own source revision. The symbol-library check retains **11 naming/sequence findings**.

The October 11 audit checks saved native data, including pins, nets, placement, geometry, parameters and footprint bindings. Both QR objects contain **195 fills** with matching primitive-count headers and a **21 mm** extent. A read-only Altium reopen confirmed the library's 195 fills in both the component group and active workboard, **571 protected records unchanged**, and **0 errors**. See the [audit method](../verification/altium/README.md) for the exact comparison scope and allowed native encoding changes.

These checks cover their recorded inputs and configured rules. Host tests do not run MCU peripherals; DRC does not qualify solder joints, connector fit, transients or NFC performance. Raster QR decoding also requires confirmation on the fabricated silkscreen.

## Reproduce firmware and mapping checks

From the repository root:

```sh
python -m unittest discover -s firmware/tests -p "test_build*.py"
python firmware/tools/build.py --build
python firmware/tools/build.py --test-host --sanitize
python firmware/tools/check_board_contract.py
```

The build requires an Arm GNU toolchain; host tests require a supported native compiler. [Firmware setup](../firmware/README.md) covers tool selection and sanitizer support. The board-contract checker uses Python's standard library, reads the root PCB and exported XML, and records input hashes and negative controls.

Artifact checks reject checkout, user-home and private toolchain paths. Paths from precompiled system libraries, such as `/usr` on Linux, are allowed because compiler prefix maps cannot rewrite their existing debug records.

After KiCad hardware edits, refill zones, save the board, regenerate the XML netlist and rerun ERC/DRC and the board-contract check before exporting fabrication files. For Altium edits, run project Validate and PCB DRC and regenerate that revision's outputs; see [Altium](ALTIUM.md). After contact edits, regenerate QR/NDEF and rerun the contact audit in [Customization](CUSTOMIZATION.md). Keep reports and exports associated with the same source revision.

## Physical validation

Five bare laboratory boards have been ordered. Receipt, stencil/component purchases and assembly are unconfirmed. All **eight** [component-review findings](COMPONENT_REVIEW.md) remain open.

Test one assembled sample for supply current, rail stability, temperature, display and button operation, I2C timing, watchdog recovery, USB fit and transients, NFC read range, and printed QR readability. Use [Bring-up](BRINGUP.md) for inspection, programming and test procedures. Record the board revision, firmware hash, instrument settings and measured results.
