# Rev C prototype build plan

Updated 28 October 2026. The next milestone is **one measured first article of SC-REVC-LCSC-P1**. Native CAD, DRC/ERC and final manufacturing-output review are complete. Supplier acceptance and physical qualification remain open; all P01–P14 results are NOT TESTED.

## 1. Establish the exact build revision

Use [Altium Rev C](ALTIUM.md), the [controlled BOM](revc/assembly/BOM.md), [current fabrication files](../fabrication/revc/), [placement register](revc/assembly/PLACEMENT_REGISTER.md) and [drawings](revc/assembly/ASSEMBLY_REFERENCE.html). Retain the [publication manifest](../verification/revc/PUBLICATION_MANIFEST.json), which maps public copies to the verified engineering source.

Confirm the bare PCB matches Rev C before placing parts. Earlier boards, root KiCad exports and the old rear five-pad SWD row have a different scope; this record does not amend a previously fabricated board. Record the board revision/serial and actual installed MPNs and firmware hash.

## 2. Close supplier and process inputs

| Input | Required work |
|---|---|
| Bare board | Accept finished 0.8 mm thickness/tolerance, R2 profile, mask registration, plated slot dimensions and USB fit. |
| Parts | Confirm the exact 17 purchasing groups, received markings, lot/moisture information and reviewed substitutions. |
| Capacitors | Establish C1/C2 effective capacitance ≥0.50 µF after declared combined conditions and allowances. |
| Paste/stencil | Accept Top-only 0.10 mm printing process, QFN windows, thin-board support and measured reflow profile. |
| Inspection | Establish hidden-QFN-joint inspection and criteria for LED/U3 alternate lands and USB joints. |
| Programming | Prepare a fixture or temporary connections accessing Top TP7 and the four Bottom SWD contacts, with programmer target power disabled. |
| Test equipment | Confirm current-limited 5 V power, microscope, multimeter, SWD programmer, oscilloscope and suitable NFC RF equipment. |

The [fabrication specification](revc/FABRICATION_NOTES.md), [assembly manual](revc/assembly/ASSEMBLY_MANUAL.md) and [component review](COMPONENT_REVIEW.md) define the details. No supplier acceptance is currently recorded.

## 3. Assemble one controlled sample

Use 118 Top reflow parts, manually solder Bottom U1/C10 and install the five switches last after rework/compatible cleaning. Keep C30 DNP initially. Check all LED cathodes and D91 K1/A2 polarity before heating. Record paste/profile, equipment, operator and inspections. Do not populate additional cards until the first sample is accepted.

## 4. Execute P01–P14

Follow [Bring-up](BRINGUP.md) and the [qualification record](revc/electrical/QUALIFICATION_PLAN.md):

1. Inspect polarity, hidden/visible joints and unpowered rail resistance.
2. Establish rails under controlled current, then capture startup/shutdown and load response.
3. Program/verify over SWD, check every pixel/button and test game/reset/recovery behavior.
4. Measure I²C, worst-pattern current, rail behavior and temperatures within the declared input/ambient envelope.
5. Check repeated SWD recovery, input hot-plug, positive/negative system stress and regulator reverse bias using suitable fixtures.
6. Measure/tune NFC, select C30 only if supported by measurement, verify the public URL on phones and scan the fabricated QR.

Retain GCC ≤32, the 100 mA input-current screen and the 10–40 °C ambient envelope pending results. Enter PASS, FAIL or NOT TESTED with measurements and trace files. A failed result requires diagnosis/ECO and repeated affected tests, not a waiver based on digital checks.

## First-article acceptance

Accept the first article only after the exact installed variant and required measurements are documented, failures are resolved, and supplier/process requirements are accepted. DRC completion does not establish solder integrity, stability, thermal margin, ESD immunity, NFC performance or reliability. Review that evidence before increasing quantity or widening the operating envelope.
