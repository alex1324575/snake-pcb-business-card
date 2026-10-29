# Rev C 3D assembly previews

The README shows the current Rev C layout populated with **125 fitted parts: 123 Top and two Bottom**. U1 and C10 are on Bottom. C30 remains unpopulated; printed antenna, QR artwork and test pads have no component bodies.

| View | Image |
|---|---|
| Rotation | [Looping assembly view](../images/revc-3d-rotation.gif) |
| Isometric | [Populated board](../images/revc-3d-isometric.png) |
| Top | [Top assembly](../images/revc-3d-top.png) |
| Bottom | [Bottom assembly](../images/revc-3d-bottom.png) |

## Source and placement

The published native Altium PCB was imported into a temporary KiCad 10.0.6 rendering copy. All fitted component origins, rotations and sides were compared with the [controlled placement data](assembly/assembly_variant.json). The import also retained the 85.6 x 54 mm R2 outline, 361 copper pads, nine separate U2 paste windows, 189 vias, two coil holes and four USB shell slots. The coordinate translation introduced by the importer is recorded in the [render record](../../verification/revc/RENDER_3D.json).

The rendering copy uses black solder mask, white silkscreen, ENIG appearance and open via masks. The three views were ray-traced with KiCad's high-quality renderer. Native Altium sources and the verified manufacturing files were not edited. Layer-inspection views remain in the [CAM review](../../verification/revc/CAM_VISUAL_REVIEW.md).

## Rotation animation

The GIF uses 96 rendered views of the same populated board, with a full rotation about Y and fixed X = -22° and Z = 15°. An orthographic camera keeps the board at a consistent scale. Frames share a 256-color palette and run at 80 ms per frame; the front and back views each remain visible for 400 ms. The 8.32-second sequence loops continuously.

This is an assembly turntable with unlit LEDs. It does not show firmware execution or an assembled-board test. Render settings, source hashes and output checks are recorded in the [animation record](../../verification/revc/RENDER_GIF.json).

The separate [gameplay demo](GAMEPLAY_DEMO.md) shows captured host-firmware PWM output on the Rev C board and a synchronized enlarged matrix.

## Model scope

| Parts | Model source and scope |
|---|---|
| LEDs, resistors, capacitors | KiCad generic 0603 package models; LEDs shown unlit |
| U1, U3, U5, D91 | KiCad LQFP32, SOT-23-5, SC-70 and SOD523 package models |
| U2 | Generic 5 x 5 mm, 0.4 mm-pitch QFN40 model; model's hidden exposed pad is 3.6 mm, while the controlled PCB pad is 3.4 mm |
| SW1-SW5 | KiCad SKQG package model |
| J1 | Project's approximate USB connector envelope, based on the 8.94 x 6.90 x 3.16 mm drawing |
| U4 | Project's approximate 1.7 x 1.4 x 0.5 mm UFDFPN body envelope |

These are assembly visualizations. Generic packages and project envelope models do not establish exact manufacturer geometry, solder fillets, mechanical fit or assembly yield. The [BOM](assembly/BOM.md), native footprints and manufacturer drawings control part selection and dimensional review. No physical assembly result is implied.

KiCad model attribution and the local envelope-model scope are covered by [third-party notices](../../THIRD_PARTY_NOTICES.md). Exact source, model and image hashes are in the render record and [publication manifest](../../verification/revc/PUBLICATION_MANIFEST.json).
