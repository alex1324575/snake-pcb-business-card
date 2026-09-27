# Engineering plan: from design to a working card

Updated 2026-09-27. The current milestone is **pre-prototype review**. A clean KiCad check and passing firmware CI establish useful digital evidence; a board has not yet been assembled or powered.

## Typical development sequence

| Stage | Deliverable | Exit evidence |
|---|---|---|
| Requirements | Functions, dimensions, power source, use conditions, cost and measurable acceptance targets | A written scope that can be tested |
| Circuit design | Exact components, calculations, schematic, debug access | Datasheet review and electrical margins, with unresolved items recorded |
| PCB and manufacturing review | Layout, footprints, stackup, assembly process | ERC/DRC plus human electrical/mechanical review and factory feedback |
| Engineering prototypes | A small, traceable sample batch | Inspection, controlled first power-up, firmware programming and subsystem tests |
| Design validation | Revised board and measured acceptance report | Function, thermal, mechanical, power-cycle and relevant environmental/ESD requirements met |
| Production preparation | Approved files, sources, programming/test fixture and inspection criteria | Pilot build with repeatable assembly and test results |

A personal portfolio project can stop after a well-documented working prototype. Selling or distributing a finished product adds requirements such as regulatory review, repeatability, sourcing, support and production testing. The applicable requirements depend on intended use and sales markets; this project does not claim compliance certification.

## Next actions for this revision

### 1. Close the pre-assembly findings

Use [Component review](COMPONENT_REVIEW.md) as the issue register. Resolve these before approving an assembly order:

- **Switch-area vias:** move the SW2/SW5 vias or obtain a documented manufacturer/assembler-approved process for the deviation. Generic solder-mask tenting is not evidence that the switch's flux-ingress restriction is satisfied.
- **Regulator capacitors:** establish exact-part effective capacitance for C1/C2 above 0.47 uF after bias, tolerance, temperature and aging. The selected 35 V rating is useful voltage margin, but does not replace that evidence.
- **Assembly process:** agree on U2 exposed-pad vias, paste/stencil treatment, existing land-pattern deviations, USB plated slots, and the switches' soldering/no-wash restrictions.
- **Sources and test access:** confirm exact MPN availability and substitutions, the SWD fixture, test equipment and who will perform first-board measurements.

The input ESD/LDO protection relationship also needs a documented validation plan: component clamp ratings do not guarantee that the assembled regulator survives a transient. A controlled engineering sample may be used to investigate this open item; it is not a qualified product release.

### 2. Prepare one consistent manufacturing package

After any layout fixes, regenerate the schematic PDF, BOM, Gerber/drill files, checks and hashes from the same source revision. Add an assembly drawing and component position file (CPL/pick-and-place) for the chosen assembler. Verify pin 1, LED polarity and connector orientation in the factory's preview; KiCad rotation conventions are not universal factory conventions.

The generic [procurement BOM](../exports/Procurement_BOM.csv) is manufacturer-part-number based. A factory may additionally require its own stock identifiers and file format. For example, JLCPCB documents its [BOM requirements](https://jlcpcb.com/help/article/bill-of-materials-for-pcb-assembly) and [BOM/CPL matching checks](https://jlcpcb.com/help/article/common-bom-and-cpl-matching-issues-and-explanations). No factory upload, order or payment has been performed.

Request fabrication and assembly feedback before placing the sample order. Record any approved departures from the component drawings. Keep C30 unpopulated and distinguish the printed antenna/test pads from purchased components.

### 3. Build a small sample batch

For this card, a practical first batch is **3-5 engineering samples**, subject to the assembler's minimum quantity and cost. Inspect and test the first assembled unit before committing more units where the process permits. Factory assembly is worth considering because the board has 90 LEDs and a fine-pitch QFN; it is a project recommendation, not a claim that hand assembly is impossible.

Prepare a current-limited 5 V source with a suitable USB-C connection, multimeter, ST-LINK/SWD fixture and oscilloscope. A logic analyzer helps debug I2C transactions, but an oscilloscope is needed to assess analog edges, ripple and transients. NFC tuning needs an appropriate RF measurement setup or outside lab support.

### 4. Bring up one subsystem at a time

Follow [Bring-up](BRINGUP.md):

1. Inspect polarity, pin 1, solder bridges and rail-to-ground resistance with power disconnected.
2. Apply controlled, current-limited power; verify input/output rails and current before normal operation.
3. Connect SWD, identify the MCU, program and verify the firmware.
4. Test the display's pixel walk and all five buttons before testing the full game.
5. Measure worst-pattern current, supply ripple, temperatures, and I2C rise/fall at U2 (<=300 ns).
6. Exercise reset, power cycling, watchdog and fault recovery.
7. Tune and program NFC, then verify actual QR/NFC behavior with phones.

Record measured values, instrument settings, board revision and firmware hash. A test that has not been performed stays open. Fix failures, regenerate the release package and repeat affected checks; do not rewrite acceptance limits simply to make a failed board pass.

## Definition of the next completed milestone

The next milestone is **one working, measured prototype with a closed issue log**: reliable USB power, all LEDs/buttons working, Snake stable, SWD recovery available, and the intended QR/NFC contact behavior verified. Complete the necessary board revision before ordering a larger batch.
