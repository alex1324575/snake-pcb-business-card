# Third-party notices

## KiCad library data

The project symbol library and several footprint definitions include data derived from the KiCad community libraries. Sources include the official [symbol library](https://gitlab.com/kicad/libraries/kicad-symbols) and [footprint library](https://gitlab.com/kicad/libraries/kicad-footprints). Project changes include selected symbols, reviewed pin assignments, manufacturer-based land patterns, and project-specific metadata.

KiCad library data is distributed under **CC BY-SA 4.0 with the KiCad design exception**. The original [KiCad library license](library/LICENSE_KICAD.md) is included. See the [official licensing explanation](https://www.kicad.org/libraries/license/) for the distinction between library collections and electronic designs using those libraries. Attribution for the upstream library data belongs to the KiCad community and its contributors.

Standard component models used to produce the KiCad preview images come from the installed KiCad libraries. The two small envelope models under `library/3dmodels/` are simplified project geometry, not manufacturer tolerance models. The rendered images show the example PCB design and do not represent a photographed or tested assembly.

Current [Rev C 3D previews](docs/revc/3D_VISUALIZATION.md) use these package/envelope models on a rendering-only import of the native Altium layout. Their model inventory and exact hashes are recorded in [RENDER_3D.json](verification/revc/RENDER_3D.json). External STEP models are referenced by name and are not redistributed in this repository.

## Arm GNU runtime

Firmware images link the Arm GNU toolchain's newlib-nano routines and libgcc helpers. The included [toolchain license collection](firmware/references/ARM_GNU_TOOLCHAIN_LICENSES.txt) preserves their notices, including the GCC Runtime Library Exception and newlib terms. [Toolchain notes](firmware/references/TOOLCHAIN.md) describe the exact build version and provenance. No compiler installer or unrelated tool executable is bundled.

## Manufacturer documentation

Component names and trademarks belong to their respective owners. Datasheet references are provided as links in the design notes; third-party datasheet PDFs are not redistributed in this repository.
