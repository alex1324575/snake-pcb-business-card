# Prototype build plan

Updated 2026-10-12.

Five bare development boards have been ordered for laboratory assembly. Receipt, stencil/component purchases and assembly are unconfirmed. The next milestone is **one working, measured card**. The [Altium laboratory variant](ALTIUM.md) has separate files from the KiCad baseline and its fabrication exports; its [saved-file audit](../verification/altium/public_native_audit.json) passed on October 12.

## 1. Resolve the assembly inputs

The [component review](COMPONENT_REVIEW.md) is the issue register. All eight items remain open:

| Issue | Required work |
|---|---|
| CAP-01 | Establish C1/C2 effective capacitance above 0.47 uF after bias, tolerance, temperature and aging. |
| ASM-01 | Resolve switch-area vias. The laboratory layout moves four signal vias but retains an unapproved ground via near SW5. Document the process and inspect the first sample. |
| ASM-02 | Agree on U2 exposed-pad via treatment, paste and reflow profile. The proposed 0.10 mm top stencil has nine windows; hidden-joint quality remains unverified. |
| ASM-03 | Review land-pattern deviations, USB plated slots and connector fit on the thin board. |
| PWR-01 | Define startup, hot-plug and ESD tests for the complete input network; component clamp ratings alone do not establish regulator survival. |
| ELEC-01 | Measure display current, rail stability, temperature and I2C edges. Retain GCC 32; U2 rise/fall must be <=300 ns. |
| RF-01 | Measure/tune the antenna, select C30 if needed, program the tag and test phone read range. |
| SRC-01 | Confirm exact MPNs, quantities and availability. Review substitutions before use. |

Resolve CAP-01 and the assembly deviations before approving the assembly process. Complete reflow, rework, cleaning and drying before hand-soldering switches last. Confirm the SWD fixture, equipment and first-board test plan.

## 2. Freeze the sample's file set

Use one source revision for the schematic PDF, BOM, Gerber/drill files, stencil CAM, checks and hashes. Add an assembly drawing and placement file if using factory assembly. Verify pin 1, LED polarity and connector orientation in the factory preview.

The [procurement BOM](../exports/Procurement_BOM.csv) records baseline MPN selections. Record sourcing alternatives in a revised BOM and identify the parts installed on each sample. JLCPCB's [BOM requirements](https://jlcpcb.com/help/article/bill-of-materials-for-pcb-assembly) and [BOM/CPL matching guidance](https://jlcpcb.com/help/article/common-bom-and-cpl-matching-issues-and-explanations) apply to a factory assembly order.

Keep C30 unpopulated until NFC measurements support a value. The antenna and test pads are PCB features. Existing KiCad Gerbers remain KiCad exports; regenerate fabrication files from Altium if that is the chosen source.

## 3. Assemble and test one card

Confirm the delivered parts, paste, stencil and equipment before populating the remaining four boards. The 90 LEDs and QFN require placement and joint inspection time. Prepare a current-limited 5 V source, USB-C connection, multimeter, ST-LINK/SWD fixture and oscilloscope. Use an appropriate RF setup for NFC tuning.

Follow [Bring-up](BRINGUP.md):

1. Inspect orientation, pin 1, solder bridges and rail-to-ground resistance with power disconnected.
2. Apply current-limited power and check both rails and current.
3. Identify the MCU over SWD, program and verify firmware.
4. Run the display pixel walk and test all five buttons before the game.
5. Measure worst-pattern current, rail ripple, temperatures and I2C edges at U2.
6. Test reset, power cycling, watchdog and fault recovery.
7. Tune/program NFC and verify QR/NFC contact behavior with phones.

Record board revision, installed MPNs, firmware hash, instrument settings and results. Rework or revise failed items, then repeat affected checks. Populate the remaining cards after the first sample is accepted.

## Acceptance

The sample is complete when USB power is reliable, every LED and button works, Snake is stable, SWD recovery is available, contact behavior is verified, and the issue log contains the required measurements and dispositions. Review that evidence before ordering a larger batch.
