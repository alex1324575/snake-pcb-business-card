# Make your own card

> **Historical KiCad editing workflow.** These scripts modify the root KiCad design and its contact assets; they do not modify Altium Rev C. For Rev C, edit the [native Altium project](ALTIUM.md), deliberately update both the placed QR artwork and its library model, regenerate matching outputs and repeat source/CAM/contact checks. Current public Rev C remains ALEX XU / example.com.

The public example uses **ALEX XU / Electrical Engineer**, startup initials **AX**, and **https://example.com/**. Keep identity artwork, QR content and NFC content synchronized when adapting it.

This workflow edits the KiCad project. The contact scripts use `pcbnew` and do not update the [Altium project](ALTIUM.md).

## 1. Edit the visible identity

Open the root KiCad project and edit the front/back text in the PCB editor. Preserve silkscreen clearance around pads, board edges and the QR quiet zone. Change drawing-sheet metadata if you want your name to appear in schematic exports. The native `.kicad_pcb` is the editable routed design.

Keep the antenna area free of added copper, metal lettering or mask openings. Check the front and back views after editing; bottom-side text and artwork must be readable from the physical back of the card.

## 2. Replace QR and NFC content together

The contact generator is [tools/contact_assets.py](../tools/contact_assets.py). Set its URI to your intended HTTPS address and review its name/text settings before running it. Preserve the public example files as a reference in version control.

From the repository root:

```sh
python tools/contact_assets.py
```

The generator needs its documented image/QR dependencies. Then use **the Python environment supplied with KiCad**, which includes `pcbnew`, to update the native QR footprint:

```sh
<kicad-python> tools/update_contact_board.py
python tools/audit_contact_native.py --kicad-python <kicad-python>
```

Replace `<kicad-python>` with the executable for your installation; the angle-bracket text is a placeholder, not a literal command. Check the regenerated contact files and decoded URI. Longer addresses can require more QR modules within the same 21 mm square, reducing each module's physical size. Reassess silkscreen manufacturability and sample readability after any such change.

The data-file names use `Example_` by convention; the decoded content determines the actual destination. If you rename files, update the generator, audit and documentation links together. The NFC data must still be written to the physical tag separately; rebuilding firmware does not write NFC.

## 3. Adapt the startup initials or game

Startup glyphs and display states live in [firmware/src/app.c](../firmware/src/app.c). Game rules and timing live in [snake.c](../firmware/src/snake.c). The monochrome 10 × 9 display leaves little space, so verify new glyphs at the actual pixel resolution.

Rebuild and run host tests using the [firmware instructions](../firmware/README.md). Preserve the driver current ceiling while establishing the electrical baseline. A different LED type, R24 value, matrix size or GPIO assignment is a hardware/firmware change requiring another mapping and current review.

## 4. Refresh the deliverable

1. Refill copper zones, save, and run KiCad ERC/DRC and schematic/PCB parity checks.
2. Refresh the schematic PDF, XML netlist, BOM and fabrication outputs from the edited project.
3. Run the firmware board-contract checker against those current files.
4. Refresh front/back/isometric renders and run the contact audit.
5. Build a sample and follow [Bring-up](BRINGUP.md), including actual QR and NFC checks.

Keep the project revision and generated reports together. If a change intentionally alters a checked hardware assumption, update the implementation and checker after reviewing the new circuit; do not merely bypass the check.
