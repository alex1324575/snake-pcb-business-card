# Rev C verification

The recorded native checks were performed on the saved engineering source on 28 October 2026. PCB DRC reports zero violations, warnings and waivers, with all 43 stored rules unchanged. Schematic validation reports zero errors and two reviewed reset-bias warnings. See [validation review](NATIVE_VALIDATION_REVIEW.md), [CAM review](CAM_VISUAL_REVIEW.md) and [final status](FINAL_CHECK_STATUS.json).

## Publication provenance

[PUBLICATION_MANIFEST.json](PUBLICATION_MANIFEST.json) maps verified engineering originals to the exact published files. Original report hash fields continue to identify those originals. Public hashes differ where local metadata, paths, report callbacks or PDF author information were removed.

- SchDoc and SchLib named streams are byte-identical to the verified source.
- PCB and footprint-library changes are limited to three host-path settings. Every other named stream, including routing, pads, component placement and all 43 rules, is byte-identical.
- The public project uses relative output-document settings and omits generated-document sidebar entries. Compiler settings are retained.
- All 11 machine files are byte-identical to the reviewed native exports.
- The schematic PDF retains its vector page content, 135 annotations and navigation. Private author metadata was removed; before/after renders are pixel-identical.

Container equivalence and publication checks are separate from native ERC/DRC. No additional native run on the repacked public copies is claimed.

Run the publication integrity check from the repository root:

```sh
python tools/check_revc_release.py
```

The check verifies manifest hashes, machine-file provenance, project document paths and local Markdown links. It does not execute CAD tools or hardware tests. Use the [qualification plan](../../docs/revc/electrical/QUALIFICATION_PLAN.md) to record first-article results; P01-P14 remain **NOT TESTED**, and supplier process acceptance remains pending.
