# Fabrication files

## Current Altium Rev C

Use [`revc/`](revc/) with the [Rev C fabrication specification](../docs/revc/FABRICATION_NOTES.md). It contains the final nine Gerbers and two plated-hole/slot NC files exported on 28 October 2026. The [actual CAM review](../verification/revc/CAM_VISUAL_REVIEW.md), [drill audit](../verification/revc/native_drill_audit.json) and [publication manifest](../verification/revc/PUBLICATION_MANIFEST.json) identify the checked files.

Rev C requires 191 round plated holes and four 1.2 × 0.5 mm horizontal plated USB shell slots; no NPTH file is required. Preserve common coordinates, units and scale; do not manually mirror Bottom files. Obtain supplier acceptance of thickness/tolerance, slots and stencil process before fabrication approval. Physical qualification remains open.

## Historical KiCad files

The loose `SnakeBusinessCard-*` files and `drill_report.txt` in this directory belong to the historical root KiCad revision. They have not been regenerated from Altium Rev C. Keep their entire set together; do not mix them with `revc/` or use their via, paste or debug-layout assumptions for the current build.
