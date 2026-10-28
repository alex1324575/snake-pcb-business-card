# Snake Card Rev C — prototype assembly procedure

Build specification: **SC-REVC-LCSC-P1**, public example ALEX XU. Use [the exact BOM](BOM.md), [placement register](PLACEMENT_REGISTER.md) and [searchable drawing](ASSEMBLY_REFERENCE.html) together. This procedure applies to Rev C; it does not change the already ordered Y4 board or its process.

## Build record and prerequisites

Assign a board serial number. Record the PCB revision and source hash, actual installed MPNs and lot codes, paste alloy/lot/expiry, stencil revision and thickness if used, equipment, operator and date. Verify the physical board matches the drawing before applying paste. Hold assembly if the board outline, pad positions, connector geometry or revision differs.

There are **125 fitted parts: 123 Top and 2 Bottom**. Top contains 118 reflowed parts and five manually soldered switches. Bottom contains U1 and C10, soldered manually after Top reflow. C30 stays unpopulated. The antenna, QR and probe pads are PCB features.

Use a microscope, ESD-safe tweezers, temperature-controlled iron, flux compatible with the components, fume extraction and controlled reflow equipment. U2 is a QFN with a hidden exposed pad. It needs a controlled paste print and reflow process; a visible perimeter fillet cannot establish that the center pad is soldered. X-ray or another qualified inspection process is required for product release. Retain component moisture labels and observe their floor life and bake instructions.

## Orientation checks before paste

| Item | Placement requirement |
|---|---|
| D1–D90 | Physical cathode faces **left when viewing Top**, toward PCB pad 1 / ROW. Confirm one received KENTO LED with a current-limited tester before placing the array; supplier terminal numbering differs from the PCB. |
| U1 | Bottom LQFP-32. Match package pin-1 mark to the red pin-1 pad on the mirrored Bottom drawing. Do not orient it by the Top-view coordinate table alone. |
| U2 | Top QFN-40. Match pin 1 and center pad 41 to GND. Preserve the nine center paste windows. |
| U3 | Texas Instruments **TPS7A2433DBVR**. Pin 1 is IN, pin 2 GND, pin 3 EN, pin 4 NC, pin 5 OUT. Match the exact selected MPN; this Rev C ECO supersedes the original TLV755 part. |
| U4 | NFC pins 3/4 connect to the coil. Pins 1, 2 and center pad 5 are NC; do not bridge the center to GND. |
| U5 | Use the exact three-pin DCK part. Follow the pin-1 pad marker and register; package size alone is insufficient. |
| D91 | Nexperia **PESD5Z6.0,115**, a polarized unidirectional TVS. **Pin 1 cathode K goes to VBUS_5V; pin 2 anode A goes to GND.** Match the package cathode mark to the drawing's red pad 1. Inspect the received manufacturer marking under the microscope; do not place it using the old bidirectional-device instruction. |
| Capacitors/resistors | Selected ceramic capacitors and chip resistors are not polarized. Verify value, package and reference. |
| SW1–SW5 / J1 | Align mechanical body and pads to the drawing. Preserve the connector opening and plated shell slots. |

The KENTO land pattern differs from its recommended land dimensions. Treat the first board as a prototype process trial. Inspect wetting, end-cap overlap, alignment and solder strength before duplicating the build.

## Top assembly

1. Inspect bare-board mask registration, pad damage and debris under magnification. Check no short from VBUS to GND or +3V3 to GND. Capacitor charging can make resistance readings change; investigate a persistent near-zero reading.
2. Support the 0.8 mm board flat. Print Top paste using the final Rev C paste data. Do not print the Bottom at this stage. Inspect every QFN aperture, the nine exposed-pad windows and the LED array for skips, bridges and excess paste.
3. Place U2, U3, U4 and U5, then passives and D91, followed by the LED array and J1. Use the placement register to check each reference. Leave C30 and switches clear. Check all 90 cathodes before heating.
4. Reflow once using the paste supplier's profile within every fitted component's limits. Measure the profile on a representative loaded board. Record ramp, soak, peak and time above liquidus; do not infer an approved profile from the oven's display.
5. Cool without moving components. Inspect for bridges, tombstones, skew, insufficient wetting and loose connector joints. Inspect the QFN with the qualified hidden-joint process. Rework only affected joints with controlled heat.

## Bottom and manual assembly

Use a fixture supporting the board without pressing against Top LEDs or the NFC area. Protect the populated Top from excess heat. Flip the board left-to-right and compare with the actual underside drawing.

1. Apply flux to U1 lands. Place U1 with its pin-1 mark at the drawing's red pad. Tack two opposite corners and inspect alignment on all four sides before drag-soldering. Use minimal solder; remove bridges with flux and braid without levering the fine leads.
2. Solder C10 beside U1. Check VDD-to-GND for a persistent short and inspect every MCU lead under magnification. Avoid uncontrolled whole-board second reflow: this manual process was selected to keep the first-side assembly supported.
3. Fit SW1–SW5 and solder their terminals within the switch manufacturer's time/temperature limits. Prevent liquid flux or cleaner entering the switch mechanism. Verify all five actuate freely.
4. Complete J1 shell joints as required; inspect its contact pins and plated slots. Clean only with a method allowed for all fitted parts. Dry and inspect before power.

## Programming connection

Use a fixture that supports the board at its edges, leaves both faces accessible and holds the probe tips without hand pressure. Short temporary leads attached to the named test pads are an acceptable first-prototype method. A production jig needs its own contact/access validation.

| Programmer signal | PCB contact | Face | Native X / Y, mm |
|---|---|---|---|
| SWDIO | TP6 | Bottom | 137.9982 / 12.2936 |
| SWCLK | TP7 | **Top** | 142.6464 / 9.4996 |
| NRST | TP8 | Bottom | 157.9880 / 2.9972 |
| Target voltage reference | TP4, +3V3 | Bottom | 145.9992 / 1.5240 |
| Ground | TP5, GND | Bottom | 148.9964 / 2.9972 |

Power through the controlled 5 V USB input for programming. Connect +3V3 only to the programmer's **target-voltage sense input**. Disable programmer target-power output; feeding +3V3 directly can drive reverse current through U3. Connect GND first and disconnect it last. Start SWD at 100 kHz; use connect-under-reset if firmware recovery is needed.

## First power and acceptance

Follow [the electrical qualification plan](../electrical/QUALIFICATION_PLAN.md). Start with display disabled and a current-limited 5 V supply. Establish the 3.3 V rail before programming or enabling the matrix. A current-limit event, abnormal rail or rising temperature is a stop condition requiring diagnosis.

After verified programming, check all 90 pixel positions, five buttons, reset, watchdog recovery and an extended worst-pattern thermal run. Record results and instrument traces. NFC tuning, transient protection and capacitor stability remain release gates requiring measurement. Approve the first board's build record before assembling the remaining cards.

The selected regulator requires **C1 and C2 effective capacitance of at least 0.50 uF** after the declared bias, tolerance, temperature, aging and measurement allowances. Do not infer that result from their nominal 1 uF labels. The U3/D91 ECO corrects the known positive voltage-rating mismatch; it does not establish system ESD performance. In particular, forward conduction of the new unidirectional TVS does not guarantee the regulator's -0.3 V input limit during a negative event. Keep all unexecuted tests marked NOT TESTED.
