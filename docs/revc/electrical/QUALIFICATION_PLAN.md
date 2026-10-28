# Rev C electrical qualification and acceptance record

Scope: controlled first-article testing of the public Rev C assembly. **No physical results have been entered.** The schematic/PCB graph and firmware host tests establish connections and software behavior; they do not establish voltages, temperature, solder integrity or RF performance.

Prototype operating envelope to verify: regulated 5 V input, 4.75–5.25 V at the connector under load; ambient 10–40 °C, dry indoor use; R24 = 20 kohm; firmware global-current register GCC ≤32. A wider envelope requires another qualification. These are project acceptance limits, not a claim of USB or environmental certification.

Applied native power ECO for this qualification: **U3 TPS7A2433DBVR (LCSC C2866134)** and **D91 Nexperia PESD5Z6.0,115 (LCSC C461166)**. Saved SchDoc/SchLib procurement readback and the selected BOM verify both replacements; the PCB audit verifies retained physical geometry and rules. See the [implementation record](POWER_ECO_RECOMMENDATION.md) for evidence. Apply the checks below to that exact installed variant and bind the final release manifest plus actual installed-part inspection to each physical test record. A CAD implementation does not establish installed parts or hardware qualification. Purchased personal Y4 boards retain their own revision and assembly instructions.

## Required records

Record board serial, CAD revision/hash, firmware BIN/hash, installed BOM variant, component lots, instruments/probes and calibration status, ambient temperature, source voltage/current limit, actual readings, trace filenames, operator and date. Each result is PASS, FAIL or NOT TESTED. A blank result is not a pass. Repeat affected tests after rework or component/firmware changes.

## Safe staged bring-up

1. With power removed, inspect polarity, QFN and MCU joints. Check VBUS–GND and +3V3–GND resistance in both probe polarities and observe charging. A persistent near-zero reading requires diagnosis before power. Verify no unintended short on the NFC NC pads or USB CC lines.
2. Start a regulated 5 V source with a 20 mA current limit, display disabled and no external +3V3 source. This limit is a fault-detection starting point, not a guaranteed normal operating current. If it enters constant-current mode, switch off and diagnose rather than interpreting the collapsed rail as regulator failure. Increase the limit only after inspection establishes the intended load.
3. Measure input at U3 IN and output at U3 OUT and U1 VDD with short probe ground springs. Require steady +3V3 within **3.135–3.465 V**, no sustained oscillation, no unexpected heating and no current-limit event. Capture startup and shutdown at 4.75, 5.00 and 5.25 V.
4. Connect SWD using the assembly manual, with programmer target power disabled. Read device identity, program, verify flash, reset and read back the firmware identity. Repeat connect-under-reset recovery. Never connect two independent rail-power sources together.
5. Enable the driver at low brightness first. Check 7-bit I2C address 0x50, shutdown and restart. Verify all 90 pixels individually before the all-on pattern. Keep GCC ≤32 even for diagnostic builds.

## Test matrix

| ID | Method / conditions | Acceptance | Evidence required | Result |
|---|---|---|---|---|
| P01 | Inspect input/output and package orientation before power | Matches exact BOM and pad map; no persistent short or visible bridge | Microscope photos and resistance readings | NOT TESTED |
| P02 | Capture 20 cold starts and 20 shutdowns at each input corner | No latchup/reset loop; steady rail 3.135–3.465 V; no excursions beyond fitted devices' absolute limits | Scope traces at U3 and U1 | NOT TESTED |
| P03 | Display off → all 90 pixels full PWM → off, GCC32 | Rail remains inside 3.135–3.465 V after settling; no sustained oscillation/reset; no supply current-limit event | U3/U2 supply and input-current traces | NOT TESTED |
| P04 | Controlled 0–100 mA external load-step characterization of the isolated 3.3 V regulator load, if a suitable fixture permits isolation | No oscillation; repeatable recovery; waveform within device absolute limits; final rail in project band | Load-step amplitude/slew, traces, exact capacitors | NOT TESTED |
| P05 | Scope I2C at U2 with actual loaded bus | Rise/fall, low/high voltage and setup/hold satisfy the configured bus speed and both device datasheets; no spurious edges | Annotated SCL/SDA captures | NOT TESTED |
| P06 | 30 min all-on PWM255/GCC32 at each input corner; repeat at 40 °C ambient using suitable equipment | No current/brightness runaway, reset or thermal shutdown; steady input below 100 mA project screen; U2/U3 case ≤60 °C project screen | Current log, ambient, thermocouple/thermal images | NOT TESTED |
| P07 | Every pixel and row/column walk | Exactly one intended location lights; no reversed LED, open/short or visible ghosting | Video / 90-position checklist | NOT TESTED |
| P08 | Five buttons, bounce, long press, simultaneous presses and game reset | Correct direction/start mapping; no invalid reversal or unintended repeated trigger | Functional log | NOT TESTED |
| P09 | I2C fault and recovery fixture; watchdog recovery | Driver disables safely on fault; bus recovery and bounded restart behave as designed | Fault method and traces | NOT TESTED |
| P10 | 20 SWD program/verify/reset cycles; recover under reset | Correct ID and verified flash every time; probe contacts mechanically repeatable | Programmer logs, fixture photos | NOT TESTED |
| P11 | Input hot-plug with representative cables and sources inside selected voltage envelope | No overstress, reset loop or unexplained current spike | High-bandwidth IN/OUT captures | NOT TESTED |
| P12 | NFC procedure below, USB off and on | Meets recorded phone/read criterion; correct public/personal URL for this variant | Tune record and phone matrix | NOT TESTED |
| P13 | Defined positive and negative system transient/ESD tests in a suitable laboratory, after the design and test severity are approved | No stress beyond fitted-part absolute limits, with measurement uncertainty included; no damage or lost function; protection margin justified for the actual waveform | Test plan, port/polarity/severity, IN/EN/OUT and ground-reference traces, post-test checks | NOT TESTED |
| P14 | Fastest representative VBUS removal with display off and the lowest supported MCU load; capture IN and OUT simultaneously | No reverse overstress; TPS7A24 OUT stays at or below min(IN +0.3 V, 20 V), including uncertainty; no externally driven +3V3 | Cable/source/switch fixture, discharge-load state and synchronized traces | NOT TESTED |

The 60 °C case and 100 mA input limits are conservative project screens. Passing them alone does not prove junction temperature, long-term reliability or standards compliance. Do not use a datasheet's JEDEC thermal resistance as a measured thermal model for this board.

## E01 — input protection coordination

The review baseline paired a 10 V specified positive clamp with a 6 V IN/EN regulator limit. The applied ECO changes U3 to TPS7A2433DBVR and D91 to unidirectional PESD5Z6.0,115. U3 retains IN1/GND2/EN3/NC4/OUT5 connections and D91 has **K1 to VBUS_5V, A2 to GND**. Use the exact manufacturer/order codes; a similarly named substitute requires another review.

TPS7A24 IN/EN absolute limits are -0.3 to 20 V. D91's maximum positive clamp is 13 V at 5 A or 18 V at 10 A, for 8/20 us pulses at 25 °C. The arithmetic headroom is 7 V or 2 V respectively; these are component-data comparisons, not guaranteed margin for layout inductance or other waveforms. A unidirectional TVS does not guarantee the regulator's -0.3 V negative limit. [TI TPS7A24, sections 5 and 6.1](https://www.ti.com/lit/ds/symlink/tps7a24.pdf), [Nexperia PESD5Z6.0, tables 2 and 6](https://assets.nexperia.com/documents/data-sheet/PESD5Z6_0.pdf).

The **known positive voltage-rating mismatch is corrected in the saved CAD and selected BOM**. Native application reports zero errors; saved SchDoc/SchLib readback verifies both replacements and pin names, and the [PCB audit](../../../verification/revc/PCB_METADATA_AUDIT.json) verifies retained lands, net identity and the unchanged frozen 43-rule stream. The [implementation and package review](POWER_ECO_RECOMMENDATION.md) records the saved-state evidence. The [source record](POWER_ECO_SOURCES.json) retains the earlier component-selection assessment. AP2205 and TLV755 are not the selected Rev C build variant. This disposition closes that specific component-rating defect; it does not close physical design qualification or certify system protection.

Before a product ESD claim, define the applicable test severity and ports and perform P13. A TVS withstand rating is not an assembled-product qualification. **System protection qualification remains NOT TESTED**, including negative stress. A failed trace requires another protection/layout ECO, not a waiver.

TPS7A24 does not limit reverse current. Keep SWD target power disabled and do not externally energize +3V3 without VBUS. Perform P14 even with USB-only operation: stored output charge and a low discharge load can create reverse bias during input collapse. If P14 fails, add appropriate protection and repeat the affected checks; normal steady 5 V operation cannot close this item. [TI TPS7A24, section 8.1.4](https://www.ti.com/lit/ds/symlink/tps7a24.pdf). **Reverse-current qualification remains NOT TESTED.**

## E02 — effective capacitance

For selected Samsung CL10B105KA8NNNC, retained typical DC-bias data gives 0.6100 uF at 5.5 V and 0.6993 uF at 3.3 V after multiplying -10% nominal tolerance and -15% temperature factors. Both exceed the selected **0.50 uF effective-capacitance floor**, by 22.0% and 39.9% respectively before unclosed aging and curve-guarantee allowances. TI's nominal capacitor recommendations explicitly assume 50% derating; the 0.50 uF floor is the corresponding project acceptance criterion. The curves do not guarantee combined bias, temperature, aging and tolerance. No guaranteed production margin is inferred.

Obtain supplier/manufacturer minimum combined-corner data or qualify the exact part/lot with a bias-capable capacitance measurement over the selected environment. Require **effective CIN at C1 and COUT at C2 ≥0.50 uF after subtracting measurement uncertainty and a declared aging allowance**. Capture P02/P03/P04 with those actual capacitors. If that margin cannot be established, select a larger exact 0603 capacitor and repeat the full assessment; nominal capacitance alone is insufficient. C23/C24 are driver bulk capacitors and require supply-transient evaluation independently of regulator stability.

Sources: [manufacturer part data](https://product.samsungsem.com/mlcc/CL10B105KA8NNN.do), [retained screening calculations](calculations.json), and [TPS7A24 sections 6.3 and 8.1.2–8.1.3](https://www.ti.com/lit/ds/symlink/tps7a24.pdf). **E02 remains open for combined-corner and board stability evidence.**

## E03 — driver thermal and supply path

Nominal peak LED current = (840 / 20,000) × (32 / 256) = **5.25 mA**. With 90 LEDs at PWM255/256 and the documented 1/12.75 scan duty, total nominal average LED current is **36.91 mA**, consistent with the previous 36.9 mA estimate. Tolerances, driver overhead and MCU current are additional. Ignoring the small PWM blanking fraction gives a conservative nominal-load screen of 37.06 mA. Assigning all 3.3 V ×37.06 mA = **0.1223 W** to the driver is deliberately pessimistic for that nominal LED contribution, but is not a complete worst-case power bound. Real LED voltage reduces driver dissipation; current gain, scan timing and temperature tolerances still matter.

Rev C has no via drill inside U2's exposed pad. The datasheet normally recommends a thermal-via array; its published 31 °C/W JEDEC value is not transferable to this layout. Frozen large via rules prevent inserting the old fine-pitch array. Keep the current ceiling and qualify heat flow with P06. If it fails, redesign the GND heat path using compliant geometry or lower the approved ceiling and repeat all brightness/functional tests; do not relax via rules.

The saved C23-to-U2 bulk path is approximately 19.8 mm with two vias, while the local C22 bypass path is approximately 3.06 mm with no via. Scope at the actual U2 supply pins during pattern transitions. Further local bulk placement is justified if measured droop or ringing fails P03. **E03 remains open until thermal and supply traces are accepted.**

Also qualify replacement U3 under P06. At 5.25 V input, nominal 3.3 V output and the 100 mA project load ceiling, pass-element dissipation is approximately **0.195 W** before ground-current loss. This calculation supplies a thermal test condition; it is not a measured junction temperature. Preserve the 10–40 °C prototype envelope until accepted records support a wider one.

Source: [IS31FL3737B Rev D](https://www.lumissil.com/assets/pdf/core/IS31FL3737B_DS.pdf), current equation and power/layout guidance. The firmware ceiling is an operating restriction, not proof of thermal qualification.

## R01 — NFC tuning and read acceptance

Keep C30 DNP for the first measurement. ST25TN01K nominal internal tuning capacitance is 50 pF (47.5–52.5 pF). The ideal inductance at 13.56 MHz with exactly 50 pF is approximately 2.755 uH; coil and parasitic losses/capacitance prevent using this ideal calculation as a board pass.

Use a weakly coupled loop and VNA/appropriate NFC characterization fixture so the measurement probe does not materially detune the board. Record fixture position, calibration, board power state, fitted parts and measured resonance. If resonance is above target, calculate a provisional added parallel capacitor using measured circuit parameters, populate C30 with the selected exact value, and remeasure. If resonance is below target, adding C30 cannot raise it; revise the coil/parasitics. Verify field-powered operation and the chip's RF voltage conditions after tuning.

For each of at least one iOS and one Android phone, record model/OS/case, power off/on state, approach direction and reliable tap position. Require 10 consecutive reads at the declared tap position in each power state, correct NDEF URL, and successful repeated read after power cycling. Declare a reliable distance only from recorded trials; no minimum distance is claimed here. Do not lock NFC contents before verifying the intended identity and URL. Public and personal releases need deliberate, separate content checks.

Source: [ST25TN01K datasheet](https://www.st.com/resource/en/datasheet/st25tn01k.pdf). **R01 remains open until tuning and phone results are recorded.**
