# Rev C fabrication and assembly

Updated 28 October 2026. This guide applies to **Altium Rev C / SC-REVC-LCSC-P1**. Use the current [Gerber/NC files](../fabrication/revc/), [BOM](revc/assembly/BOM.md), [placement register](revc/assembly/PLACEMENT_REGISTER.md) and [drawings](revc/assembly/ASSEMBLY_REFERENCE.html) together. Earlier KiCad BOM, stencil and debug-pad instructions describe a different revision.

Native validation and final CAM review are complete. Supplier process acceptance and every physical qualification test remain open. A zero DRC result does not approve a soldering process.

## Fabrication inputs

The [fabrication specification](revc/FABRICATION_NOTES.md) is the detailed acceptance document.

| Parameter | Rev C requirement |
|---|---|
| Profile | 85.6 × 54 mm, four R2 corners |
| Material / layers | FR-4, two copper layers |
| Nominal thickness | 0.8 mm; supplier tolerance and first-article USB fit require acceptance |
| Copper / finish | Nominal 1 oz each face / ENIG |
| Appearance | Black solder mask, white silkscreen, ALEX XU, REV C / AX-01 |
| Standard vias | 189 vias: 1.27 mm land / 0.7112 mm hole, open on both faces under the preserved rules |
| Other round holes | Two printed-coil holes, 0.3 mm diameter |
| Plated slots | Four horizontal J1 shell slots, 1.2 × 0.5 mm overall |
| NPTH | None |
| Stencil process | Top only, assessed at 0.10 mm; supplier printing/process acceptance pending |

Preserve the common millimeter origin and scale. Do not manually mirror Bottom manufacturing layers. Obtain supplier acceptance for finished thickness, slot plating/tolerances and stencil requirements. Confirm the public QR artwork in the actual fabrication preview.

## Controlled assembly inputs

- [BOM](revc/assembly/BOM.md): 17 purchasing groups and 125 fitted parts per board.
- [Per-reference placement register](revc/assembly/PLACEMENT_REGISTER.md): selected MPN, side, native origin and rotation.
- [Searchable drawings](revc/assembly/ASSEMBLY_REFERENCE.html): side-correct pad/reference views.
- [Detailed assembly manual](revc/assembly/ASSEMBLY_MANUAL.md): orientation, tools, sequence and programming access.
- [Library override policy](revc/assembly/LIBRARY_OVERRIDE_POLICY.md): intentional placed-instance geometry and update controls.

There are **123 Top parts and two Bottom parts**. The defined prototype process reflows 118 Top parts, manually installs Bottom U1/C10, then hand-solders the five Top switches. C30 stays DNP until NFC measurements establish a value. The printed antenna, QR artwork and probe features are not purchased parts.

## Orientation checks

Use the actual selected manufacturer's markings and the controlled drawings. A legacy library identifier does not select a purchasing part.

| Item | Rev C placement requirement |
|---|---|
| D1–D90, KENTO KT-0603R | Physical cathode faces left in Top view, toward PCB pad 1 / ROW. Supplier terminal numbering differs from the CAD numbering; test one received LED before placing the array. |
| U1 | Bottom LQFP-32; match package pin 1 to the mirrored Bottom drawing. |
| U2 | Top QFN-40; match pin 1; exposed pad 41 is GND. Retain nine separate paste windows. |
| U3, TPS7A2433DBVR | IN1, GND2, EN3, NC4, OUT5; use the exact selected order code. |
| D91, PESD5Z6.0,115 | **Polarized:** cathode K / pad 1 to VBUS_5V; anode A / pad 2 to GND. The earlier bidirectional-device instruction does not apply. |
| U4 | Coil on pins 3/4. Pins 1/2 and center pad 5 are NC; do not ground the center pad. |
| U5, TPD2E2U06QDCKRQ1 | Match exact DCK package and pin-1 marker. |
| Resistors / ceramic capacitors | Not polarized; verify value and reference. |
| SW1–SW5 / J1 | Match mechanical outline, contact lands and plated slots. |

## Process controls

1. Assign a serial number and record PCB/firmware hashes, installed MPNs/lots, paste and stencil revision, equipment and operator.
2. Inspect bare-board registration, shorts and connector geometry. Support the thin board flat.
3. Print Top paste and inspect the QFN windows, fine-pitch lands and all 90 LEDs. Do not print Bottom for this process; do not paste C30 or switches.
4. Place Top reflow parts, check polarity, then reflow using a measured profile within paste and component limits.
5. Inspect and rework before manual assembly. U2's hidden joint requires X-ray or another qualified inspection method for product release; visible perimeter fillets do not establish exposed-pad quality.
6. Support the populated Top while soldering Bottom U1/C10. Install switches last, after reflow/rework and compatible cleaning; prevent liquid flux/cleaner entering the mechanisms.
7. Inspect every MCU lead and the USB joints, then follow [Bring-up](BRINGUP.md). Accept one controlled first article before repeating the build.

The selected LED land pattern and U3 lands are reviewed prototype deviations, not exact copies of every manufacturer's example. U2 has no via drill within its exposed pad under the frozen large-via rules. Printing yield, hidden-joint quality, heat flow and mechanical fit require first-article evidence.

## Programming access

Rev C does **not** have the earlier single rear five-pad row. TP7/SWCLK is **Top**; TP6/SWDIO, TP8/NRST, TP4/+3V3 sense and TP5/GND are Bottom. Use the [side-correct table](BRINGUP.md#2-connect-swd-on-both-faces) and drawings; a fixture must access both faces.

Power through controlled 5 V USB. Connect +3V3 only to programmer target-voltage sense and disable programmer target-power output. U3 does not provide reverse blocking. The [qualification record](revc/electrical/QUALIFICATION_PLAN.md) defines the 14 outstanding physical tests, including reverse-bias and system-transient measurements.
