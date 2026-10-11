# Fabrication and assembly

Updated 2026-10-07.

Choose the source variant before assembly. The root KiCad design, [BOM](../exports/SnakeBusinessCard_BOM.csv) and existing [fabrication directory](../fabrication/) form the baseline package. The separate [Altium laboratory variant](ALTIUM.md) has later layout and paste changes; regenerate manufacturing outputs from that variant when using it. Keep the board, purchasing BOM, placement drawing and stencil tied to the same revision. A passing digital check does not establish production readiness.

## Prototype preparation status

| Item | Recorded status |
|---|---|
| Laboratory PCB batch | Five bare boards ordered; receipt is unconfirmed |
| Components | Sourcing reviewed; purchase and receipt are unconfirmed |
| Stencil | Top-only 0.10 mm process specified; purchase and CAM approval are unconfirmed |
| Assembly and bring-up | Not completed; no measured hardware results |

The laboratory order concerns development files. The public Altium files have fresh native schematic/PCB checks recorded in [Verification](VERIFICATION.md); those checks do not approve the fabrication package or qualify the assembly process.

## PCB order parameters

| Parameter | Design value |
|---|---|
| Finished dimensions | 85.6 × 54 mm, R2 corners |
| Layers / thickness | 2 / 0.8 mm |
| Copper | 35 µm, approximately 1 oz, per side |
| Finish | ENIG |
| Solder mask / silkscreen | Graphite black / white |
| Minimum track / clearance | 0.15 / 0.15 mm |
| Standard via | 0.60 mm pad / 0.30 mm drill |
| Minimum copper-to-edge clearance | 0.30 mm; review the USB mounting region separately |
| USB plated slots | Four 0.50 × 1.20 mm slots, 1.00 × 1.70 mm copper pads |

Preserve a common origin and scale for every fabrication layer. Bottom layers should remain in their exported orientation; do not manually mirror them. Confirm that the manufacturer's CAM process recognizes the plated slots and retains the QR silkscreen geometry.

## Procurement

All 125 fitted parts now have exact manufacturer part numbers in the [procurement BOM](../exports/Procurement_BOM.csv). The [component review register](COMPONENT_REVIEW.md) covers every reference and identifies conditional selections. C1/C2 effective capacitance, switch-area vias and assembly-process decisions remain open; a complete purchasing identity is not assembly approval. Confirm supplier availability and review substitutions against the exact source documents.

Test points, the printed antenna and the QR artwork are PCB features. They are not purchased components. C30 is **do not populate** until NFC measurements establish a tuning value.

## Assembly details requiring attention

- **LED polarity:** D1–D90 use APT1608SURCK. Pad 1 is cathode; pad 2 is anode. Check the actual reel orientation against the footprint before assembly.
- **QFN exposed pad:** U2 has 0.4 mm lead pitch and a 3.4 × 3.4 mm exposed pad. The footprint uses nine paste windows and 16 ground vias below the exposed pad. Agree on stencil thickness, paste coverage and via treatment with the assembler to control solder loss into the vias. Filling/capping requirements can affect cost.
- **USB mechanics:** confirm the connector's seating, shell tabs, plated slots and plug clearance on the 0.8 mm board. The simplified render is insufficient for this check.
- **NFC area:** keep ground pours, panel copper, metallic artwork and labels out of the antenna/feed keepout. Changes here require another antenna review.
- **QR:** retain the full 21 mm square and its four-module quiet zone. Sample black-mask reflectivity, white-ink registration and phone readability before quantity production.
- **Thin-board handling:** use appropriate assembly support and handling fixtures around the connector and fine-pitch parts.
- **Switch flux ingress:** the KiCad baseline has an under-body SW2 via and a nearby SW5 via. The later laboratory layout moves four button-signal vias but retains a ground via near SW5. ASM-01 remains open for both scopes; no manufacturer approval of that retained deviation is recorded. Ordinary tenting does not establish compliance. Follow [the switch review](POWER_COMPONENT_REVIEW.md#sw1-sw5-tactile-switches).

## Polarity and orientation

Use the front/component-side view with the name upright and USB-C opening on the right. Check the actual delivered markings under magnification before reflow.

| References | Placement rule |
|---|---|
| D1-D90 | Board pad 1 is cathode / LED_ROW on the left; pad 2 is anode / LED_COL on the right. Check the selected LED's own polarity marks. Manufacturer pin numbers can differ from the CAD numbers. |
| U1, U2 | Pin-1 identifier at upper-left. U2's exposed pad is GND. |
| U3 | Three-lead side down, two-lead side up; pin 1 bottom-left. |
| U4 | Pin 1 upper-left; center pad 5 is NC. Its package connection drawing is a bottom view. |
| U5 | Single GND lead left, two IO leads right; pin 1 bottom-right for the reviewed DCK package. |
| J1 | Socket opening faces outward to the right; dry-fit all four shell anchors in the plated slots. |
| SW1-SW5 | Leads extend left/right. A 180-degree rotation preserves the circuit; a 90-degree rotation does not match the land pattern. |
| Resistors and fitted MLCCs | No electrical polarity; match value and exact BOM part. |
| D91 | Exact TPD1E10B06DYAR is bidirectional; 180-degree reversal is electrically equivalent. Keep its terminal axis aligned with the footprint. |
| C30 | Leave unpopulated until NFC measurements establish a value. |

For a screened KENTO KT-0603R alternative, the manufacturer's pin 1 is anode and pin 2 is cathode. The board still requires cathode left. Confirm one received LED against its drawing and a diode-mode check before placing the full matrix. This alternative is not a change to the baseline purchasing BOM.

## Laboratory stencil and switch sequence

The laboratory process uses a proposed **top-only, frameless, 0.10 mm stainless stencil**, with apertures taken 1:1 from that variant's F.Paste layer. Its reviewed development output has 328 apertures and no bottom apertures. C30 and SW1-SW5 are excluded; all 90 LEDs and all five U4 pads retain paste. U2 retains 40 perimeter apertures and nine exposed-pad windows. Review the actual supplier CAM before approving a stencil. These counts apply to the laboratory source, not automatically to the unchanged KiCad fabrication directory.

The nine U2 windows are 0.9 x 0.9 mm rounded apertures on 1.1 mm centers, providing approximately 62.2% exposed-pad coverage. The 0.10 mm thickness is a calculated starting point for the fine-pitch QFN. Profile the selected paste on the actual board; package temperature limits alone do not specify an oven recipe. Sixteen open ground vias intersect the exposed-pad paste windows, so solder wicking and hidden-joint quality remain open assembly risks. Tape, tenting or extra paste does not establish joint acceptance.

1. Place and reflow the components that have paste apertures. Inspect polarity, pin 1, bridges and the QFN joints; complete any necessary rework.
2. Finish required cleaning, then dry and inspect the board before mounting switches.
3. Hand-solder SW1-SW5 last with minimal suitable flux. The reviewed [Alps specification](https://tech.alpsalpine.com/cms.media/SKQGAB_KQG_719_EN_cc4e10226e.pdf) limits hand soldering to 350 C, 3 seconds continuously and a 60 W maximum iron; avoid force on the terminals.
4. Do not use water-soluble flux or wash mounted switches. Complete nearby flux-producing work before they are installed, and avoid subsequent backside wave/flow soldering.
5. Inspect and test one card's switch feel, contact behavior and electrical operation before populating the remaining boards.

This sequence addresses the documented flux-ingress mechanism. The retained nearby SW5 hole still requires a documented process decision and sample verification; the sequence is not manufacturer approval of that layout deviation.

## Prototype acceptance

Follow [Bring-up](BRINGUP.md) on the first assembled board. Measure supply rails, USB current, LED brightness and temperature; test buttons, firmware, power cycling and contact functions. Antenna tuning, phone read range, ESD behavior and connector durability remain physical validation tasks. Record results against the exact board revision and firmware image before increasing the build quantity.

The staged [engineering plan](PROTOTYPE_PLAN.md) lists the remaining pre-assembly gates and sample-test sequence.
