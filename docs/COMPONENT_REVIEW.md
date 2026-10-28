# Rev C component selection and review

Updated 28 October 2026. The current prototype variant is **SC-REVC-LCSC-P1**, with **125 fitted parts across 17 purchasing groups**. Exact selected procurement metadata was applied to the native schematic, SchLib and PCB and checked on saved files. Supplier availability, delivered-part identity and physical suitability remain separate acceptance steps.

## Controlled records

- [BOM](revc/assembly/BOM.md) and [CSV](revc/assembly/BOM.csv): exact manufacturer, MPN, fitted quantity and supplier reference.
- [Placement register](revc/assembly/PLACEMENT_REGISTER.md): per-reference side, native origin, angle and selected MPN.
- [Drawings](revc/assembly/ASSEMBLY_REFERENCE.html) and [assembly manual](revc/assembly/ASSEMBLY_MANUAL.md): side-correct orientation and process.
- [Saved-source audit](../verification/revc/FINAL_SOURCE_AUDIT.md) and [PCB metadata audit](../verification/revc/PCB_METADATA_AUDIT.json): native binding and geometry checks.
- [Power ECO](revc/electrical/POWER_ECO_RECOMMENDATION.md) and [qualification record](revc/electrical/QUALIFICATION_PLAN.md): electrical reasoning and unexecuted acceptance tests.

Root `procurement/parts.json`, earlier procurement exports and the earlier passive/power/display review documents describe the historical KiCad selection. Do not use them as the Rev C purchasing list or run a KiCad metadata sync expecting it to update the Altium project.

## Selected parts

Quantities are for one card before spares and assembly attrition.

| References | Qty fitted | Manufacturer | Exact MPN |
|---|---:|---|---|
| U1 | 1 | STMicroelectronics | STM32G030K8T6 |
| U2 | 1 | Lumissil / ISSI | IS31FL3737B-QFLS4-TR |
| U3 | 1 | Texas Instruments | TPS7A2433DBVR |
| U4 | 1 | STMicroelectronics | ST25TN01K-AFH5 |
| U5 | 1 | Texas Instruments | TPD2E2U06QDCKRQ1 |
| D91 | 1 | Nexperia | PESD5Z6.0,115 |
| D1–D90 | 90 | Hubei KENTO | KT-0603R |
| J1 | 1 | Same Sky | UJC-HP-3-SMT-TR |
| SW1–SW5 | 5 | Alps Alpine | SKQGABE010 |
| C1, C2, C23, C24 | 4 | Samsung Electro-Mechanics | CL10B105KA8NNNC |
| C10, C12, C20–C22 | 5 | Yageo | CC0603KRX7R9BB104 |
| C11 | 1 | Murata | GRM188Z71C475KE21D |
| R1, R2 | 2 | Panasonic | ERJ3EKF5101V |
| R10–R15, R23 | 7 | Panasonic | ERJ3EKF1002V |
| R20, R21 | 2 | Panasonic | ERJ3EKF2201V |
| R22 | 1 | UNI-ROYAL | 0603WAF1003T5E |
| R24 | 1 | Panasonic | ERJ3EKF2002V |

C30 is DNP. The printed antenna, test/probe features and QR are not fitted purchasing parts. Legacy geometry identifiers such as `Kingbright_APT1608SURCK` do not override the exact selected MPN and corrected component Comment.

**D91 is polarized:** pin 1 cathode to VBUS_5V, pin 2 anode to GND. The old bidirectional TI part is not selected. **LED physical cathode faces left in Top view**, toward PCB pad 1 / ROW; the selected supplier's terminal numbering differs from the CAD. Confirm received markings before placement.

## Completed corrective work

| Area | CAD/document disposition |
|---|---|
| Part identity | Selected fields, Comments, descriptions and library references synchronized and saved; native audit passed. |
| Positive protection-rating mismatch | U3/D91 ECO applied; closes the identified component-rating defect within the stated datasheet waveform/temperature boundary. |
| Outline, stack and release outputs | Saved outline/stack corrected; final DRC and actual CAM/drill review complete. |
| Assembly and SWD instructions | Controlled side-correct drawings, polarized-part instructions and two-sided SWD table provided. |

## Remaining acceptance gates

| Area | Required evidence |
|---|---|
| Supplier and delivered parts | Verify exact MPN, package marking, quantities, lot/moisture labels and all substitutions; no current stock or delivery claim is made. |
| C1/C2 capacitance | Effective CIN/COUT ≥0.50 µF after combined bias, temperature, tolerance, uncertainty and aging allowance; startup/load-step evidence. |
| U2 thermal and hidden joint | Qualified paste/reflow inspection and worst-pattern current/temperature testing; no via drill exists inside the exposed pad. |
| LED/U3 lands and thin-board connector | Supplier process acceptance and first-article solder/fit inspection of the reviewed alternate lands and plated slots. |
| Switch process | Manual installation after reflow/rework and compatible cleaning; verify solder limits, flux exclusion and operation. |
| Protection and reverse bias | Positive/negative system stress, hot-plug and fastest input-collapse measurements; no product ESD claim from component ratings. |
| MCU/display and recovery | All pixels/buttons, I²C timing, reset/watchdog recovery and repeated SWD programming. |
| NFC and QR | Measured tuning, correct written/read-back public URL, phone trials and fabricated QR readability. |

All **P01–P14 are NOT TESTED**. Earlier issue identifiers apply to their recorded historical geometry; the current acceptance record is the [Rev C qualification plan](revc/electrical/QUALIFICATION_PLAN.md). A complete MPN and a passing DRC are not physical qualification.

For a proposed substitution, record the exact manufacturer/order code and review electrical limits, pin map, package/lands, polarity, thermal behavior and assembly process. Update all affected native files and controlled records, then repeat the necessary CAD and physical checks. Do not substitute by nominal value or supplier short code alone.
