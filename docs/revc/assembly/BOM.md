# Rev C controlled assembly BOM

Variant **SC-REVC-LCSC-P1** | 27 October 2026 | 125 fitted parts per board

This selection is the prototype build specification. It does not establish delivered parts, current supplier availability, or product qualification. Exact procurement fields, comments and descriptions passed saved native SchDoc/SchLib readback; hashes are recorded in assembly_validation.json.

| References | Qty | Manufacturer | Exact MPN | LCSC | Description |
|---|---:|---|---|---|---|
| U1 | 1 | STMicroelectronics | STM32G030K8T6 | [C431631](https://www.lcsc.com/product-detail/C431631.html) | STM32G0 MCU; LQFP-32 |
| U2 | 1 | Lumissil / ISSI | IS31FL3737B-QFLS4-TR | [C2940515](https://www.lcsc.com/product-detail/C2940515.html) | LED matrix driver; QFN-40 with exposed GND pad |
| U5 | 1 | Texas Instruments | TPD2E2U06QDCKRQ1 | [C915089](https://www.lcsc.com/product-detail/C915089.html) | Dual CC-line ESD protection; DCK SC70-3 |
| D1-D90 | 90 | Hubei KENTO | KT-0603R | [C2286](https://www.lcsc.com/product-detail/C2286.html) | Red 1608 LED |
| J1 | 1 | Same Sky | UJC-HP-3-SMT-TR | [C3198015](https://www.lcsc.com/product-detail/C3198015.html) | USB-C power-only receptacle; 0.8 mm PCB |
| SW1, SW2, SW3, SW4, SW5 | 5 | Alps Alpine | SKQGABE010 | [C115351](https://www.lcsc.com/product-detail/C115351.html) | Tactile switch; manual solder after reflow |
| D91 | 1 | Nexperia | PESD5Z6.0,115 | [C461166](https://www.lcsc.com/product-detail/C461166.html) | 6 V unidirectional TVS; SOD523 |
| R10, R11, R12, R13, R14, R15, R23 | 7 | Panasonic | ERJ3EKF1002V | [C191124](https://www.lcsc.com/product-detail/C191124.html) | 10 kohm; 1%; 0603 |
| R1, R2 | 2 | Panasonic | ERJ3EKF5101V | [C123727](https://www.lcsc.com/product-detail/C123727.html) | 5.1 kohm; 1%; 0603 |
| R20, R21 | 2 | Panasonic | ERJ3EKF2201V | [C116387](https://www.lcsc.com/product-detail/C116387.html) | 2.2 kohm; 1%; 0603 |
| R24 | 1 | Panasonic | ERJ3EKF2002V | [C403057](https://www.lcsc.com/product-detail/C403057.html) | 20 kohm; 1%; 0603; driver REXT |
| U3 | 1 | Texas Instruments | TPS7A2433DBVR | [C2866134](https://www.lcsc.com/product-detail/C2866134.html) | 3.3 V LDO; DBV SOT-23-5; 18 V operating input |
| U4 | 1 | STMicroelectronics | ST25TN01K-AFH5 | [C3303589](https://www.lcsc.com/product-detail/C3303589.html) | Passive NFC tag; UFDFPN5 |
| C11 | 1 | Murata | GRM188Z71C475KE21D | [C389010](https://www.lcsc.com/product-detail/C389010.html) | 4.7 uF; 16 V; 0603; ceramic |
| C1, C2, C23, C24 | 4 | Samsung Electro-Mechanics | CL10B105KA8NNNC | [C29936](https://www.lcsc.com/product-detail/C29936.html) | 1 uF; 25 V; X7R; 0603 |
| C10, C12, C20, C21, C22 | 5 | Yageo | CC0603KRX7R9BB104 | [C14663](https://www.lcsc.com/product-detail/C14663.html) | 100 nF; 50 V; X7R; 0603 |
| R22 | 1 | UNI-ROYAL | 0603WAF1003T5E | [C25803](https://www.lcsc.com/product-detail/C25803.html) | 100 kohm; 1%; 0603 |

## Exclusions

C30 is not fitted. L1 is printed antenna copper. TP1-TP8 and RN01-RN04 are probe features. QR artwork is not a component. Solder paste, stencil and programmer are assembly tools, not fitted BOM items.

## Variant controls

Selected procurement families differing from the reviewed source are U2, U3, U5, D1-D90, D91, C1/C2/C23/C24, C10/C12/C20-C22 and R22. U3 and D91 are the 27 October power ECO. Do not infer an orderable MPN from a legacy footprint or symbol identifier such as `Kingbright_APT1608SURCK` or `CAP_1uF_35V_X7R_0603`: those retained identifiers locate controlled library geometry; the explicit MPN, corrected Comment and manufacturer fields define the selected part.

**D91 is now polarized.** Install Nexperia PESD5Z6.0,115 with pin 1 cathode on VBUS_5V and pin 2 anode on GND. Match the package cathode mark to the red pad-1 marker in the assembly drawing. The old bidirectional TI part is not the selected Rev C build.

The KENTO land recommendation differs from the retained PCB. Nominal terminal-to-land overlap was screened at 0.375 mm; this is a prototype deviation requiring first-board solder inspection and process acceptance. The part is not declared a universally interchangeable production substitute.

For any substitution, record exact manufacturer, MPN, lot/date code, package marking, quantity and reviewer. Never substitute by nominal value or supplier short code alone. If received labels disagree with this BOM, hold the affected placement until reviewed.

## Qualification notes

- **U2:** B variant; PWM implementation only; R24=20 kohm and firmware GCC <=32; board qualification pending.
- **U5:** Exact Q1 MPN; retain moisture label; board ESD test pending.
- **D1 group:** Cathode to PCB pad 1 / ROW / left on Top view; supplier numbering is reversed; first-board solder/brightness qualification pending.
- **SW1 group:** No paste apertures; observe manufacturer solder and cleaning limits.
- **D91:** Polarized: pad 1 cathode to VBUS_5V; pad 2 anode to GND. Match cathode marking to pad 1. System positive/negative ESD qualification remains open.
- **U3:** 20 V IN/EN absolute limit; pad 1 IN, 2 GND, 3 EN, 4 NC, 5 OUT. C1/C2 effective capacitance >=0.50 uF; no reverse blocking; startup/load and system ESD tests remain open.
- **U4:** Pins 1/2/5 NC; center pad 5 must not be grounded; RF tuning pending.
- **C1 group:** Exact-part combined derating and regulator stability remain qualification gates.

## Sources

[Controlled procurement record](assembly_variant.json), [saved native source audit](../../../verification/revc/FINAL_SOURCE_AUDIT.json), [pad and placement binding](../../../verification/revc/PCB_METADATA_AUDIT.json), and [U3/D91 power ECO](../electrical/POWER_ECO_RECOMMENDATION.md). Earlier sourcing and substitution assessments remain in the engineering workspace; this revision supersedes their U3/D91 selections. Supplier links identify parts; no fresh price or stock claim is made.
