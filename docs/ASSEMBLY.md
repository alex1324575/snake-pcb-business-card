# Fabrication and assembly

Use the editable KiCad design and [BOM](../exports/SnakeBusinessCard_BOM.csv) as the source of truth. The [fabrication directory](../fabrication/) contains Gerber and drill exports for prototype review. A passing digital check does not establish production readiness.

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
- **Switch flux ingress:** SW2 has an under-body via and SW5 has a nearby via. Resolve the manufacturer's through-hole restriction before assembly; ordinary tenting does not establish compliance. Follow [the switch review](POWER_COMPONENT_REVIEW.md#sw1-sw5-tactile-switches).

## Prototype acceptance

Follow [Bring-up](BRINGUP.md) on the first assembled board. Measure supply rails, USB current, LED brightness and temperature; test buttons, firmware, power cycling and contact functions. Antenna tuning, phone read range, ESD behavior and connector durability remain physical validation tasks. Record results against the exact board revision and firmware image before increasing the build quantity.

The staged [engineering plan](PROTOTYPE_PLAN.md) lists the remaining pre-assembly gates and sample-test sequence.
