# Rev C library update controls

The project uses SnakeCardNative.SchLib and SnakeCardNative.PcbLib. The prior saved audit establishes that all schematic instances have a library symbol and model, and all placed footprint pad numbers, sizes and layers match their model, accounting for Bottom mirroring. This finding does not establish equality of every graphic primitive.

The final board intentionally overrides footprint graphics to satisfy its unchanged silk-clearance rules: all 90 LEDs retain a 0.75 × 0.15 mm cathode bar while decorative body lines were removed; four test-pad circle trims and eight Bottom legend moves are board-specific. Pad geometry and net mapping are unchanged by these overrides.

Treat the saved PCB instance graphics as controlled release artwork. Before any library-update ECO, compare the proposed changes against the saved board, review all added overlay primitives, preserve the cathode bars, and run complete DRC plus actual silk/pad CAM inspection. Do not apply a global footprint update without reviewing its changes. This is a documented, intentional instance-graphics override, not an assertion that the library includes final board-specific artwork.

Keep procurement variants in the controlled assembly BOM. A footprint name identifies its land geometry; it is not evidence that a different supplier's part is qualified. Symbol and source metadata must carry the selected variant's exact MPN before a synchronized CAD release is declared. See schematic_metadata_update_plan.json for the prepared per-reference updates; those updates require native application and readback.
