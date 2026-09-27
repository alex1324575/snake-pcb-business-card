# Component procurement and review register

Updated **2026-09-27**. The complete native design contains **135 schematic references**: **125 fitted parts across 17 orderable MPNs**, **C30 left unpopulated**, and **9 PCB features** (L1 and TP1-TP8). Board-only NT1 and QR1 are fabricated copper/artwork, not purchased parts.

## Records

- [Procurement BOM](../exports/Procurement_BOM.csv): one purchasing line per selected MPN, fitted quantity, manufacturer, footprint, source and open items.
- [Per-reference review table](../verification/hardware/component_review.csv): every schematic reference, including deliberate non-purchased items.
- [Native KiCad BOM](../exports/SnakeBusinessCard_BOM.csv): exported schematic fields, including Manufacturer, MPN and Datasheet.
- [Selection catalog](../procurement/parts.json): machine-readable assignments synchronized into the schematic and PCB.
- [Passive selection and calculations](PASSIVE_REVIEW.md).
- [Power, protection, connector and switch review](POWER_COMPONENT_REVIEW.md).
- [MCU, display and NFC review](DISPLAY_MCU_NFC_REVIEW.md).

**A complete MPN is an identity, not a hardware test result.** The review status distinguishes checked documentation from open measurements and assembly decisions. Quantities are for one board before assembly attrition/spares. No parts have been ordered, stock reserved or substitutions approved.

## Procurement summary

| References | Quantity fitted | Manufacturer | Exact MPN |
|---|---:|---|---|
| U1 | 1 | STMicroelectronics | STM32G030K8T6 |
| U2 | 1 | Lumissil Microsystems | IS31FL3737-QFLS4-TR |
| U3 | 1 | Texas Instruments | TLV75533PDBVR |
| U4 | 1 | STMicroelectronics | ST25TN01K-AFH5 |
| U5 | 1 | Texas Instruments | TPD2E2U06DCKR |
| D91 | 1 | Texas Instruments | TPD1E10B06DYAR |
| D1-D90 | 90 | Kingbright | APT1608SURCK |
| J1 | 1 | Same Sky | UJC-HP-3-SMT-TR |
| SW1-SW5 | 5 | Alps Alpine | SKQGABE010 |
| C1, C2, C23, C24 | 4 | TDK | C1608X7R1V105K080AC |
| C10, C12, C20-C22 | 5 | TDK | C1608X7R1H104K080AA |
| C11 | 1 | Murata Manufacturing | GRM188Z71C475KE21D |
| R1, R2 | 2 | Panasonic Industry | ERJ3EKF5101V |
| R10-R15, R23 | 7 | Panasonic Industry | ERJ3EKF1002V |
| R20, R21 | 2 | Panasonic Industry | ERJ3EKF2201V |
| R22 | 1 | Panasonic Industry | ERJ3EKF1003V |
| R24 | 1 | Panasonic Industry | ERJ3EKF2002V |
| C30 | 0 | Not selected | DNP: determine value from NFC measurements |
| L1, TP1-TP8 | 0 purchased | PCB fabrication | No discrete part |

The 23 fitted passives retain their nominal values and 0603 footprints. Capacitor ratings increase from 10 V to 35 V (1 uF), 50 V (100 nF) and 16 V (4.7 uF); resistors are explicitly 1%. C11's purchasing specification is **X7R(Murata)**, with the manufacturer-specific temperature test condition explained in the passive review. Its compact schematic value reads `4.7u / 16V`.

## Open issue register

| ID | Affected references | Finding | Required closure / stage |
|---|---|---|---|
| CAP-01 | C1, C2 | Exact-part effective capacitance under combined operating conditions has not been established | Document minimum >0.47 uF after bias/tolerance/temperature/aging before assembly approval; confirm supply behavior on a sample |
| ASM-01 | SW2, SW5 | Under/near-switch vias conflict with the manufacturer's flux-ingress assembly restriction | Move vias or obtain documented approval of an appropriate process before assembly |
| ASM-02 | U2 | Ground vias in exposed pad and project paste windows require process selection | Assembler agreement on via filling/capping or other justified treatment and stencil before assembly |
| ASM-03 | U1, fitted passives, J1 | Some IPC lands differ from manufacturer examples; connector has plated slots on a thin board | Review lands, stencil, finished slots and mechanical fit with assembler; inspect first samples |
| PWR-01 | J1, U3, U5, D91 | ESD component ratings do not qualify the whole input network; D91 clamp may exceed U3 absolute maximum | Define test conditions before prototype use; measure startup/hot-plug/ESD stress and revise if required before product qualification |
| ELEC-01 | U1-U3, D1-D90, R20/R21/R24 | Current, rail stability, thermal margin and I2C edges are calculated, not measured | Measure the sample; U2 rise/fall <=300 ns; retain firmware GCC limit 32 |
| RF-01 | U4, L1/NT1, C30 | Antenna is not tuned and a physical tag has not been written/read | Measure/tune, provision URI, verify phones and read range; C30 stays DNP until measured selection |
| SRC-01 | All fitted MPNs | Manufacturer identities are assigned, but actual supplier availability/lead times are not reserved | Confirm approved exact parts and quantities at ordering; review each proposed substitute |

The current package is **not an assembly release or production qualification**. Open physical checks are intentional; the switch and capacitor findings must not disappear behind a passing digital report.

## Reproduce and maintain the records

```sh
python tools/procurement.py --check
```

After an intentional selection change:

```sh
python tools/procurement.py --sync
python tools/export_hardware.py --kicad-cli /path/to/kicad-cli
python tools/procurement.py --write-tables
python firmware/tools/check_board_contract.py
```

`--sync` changes selected value labels and hidden procurement fields only, and checks that the remaining native circuit/PCB content is preserved. The checker verifies complete reference coverage, population, native/ XML metadata, footprints and generated tables. It does not determine electrical suitability; the linked review documents contain that reasoning. See [Engineering plan](PROTOTYPE_PLAN.md) for the path to a measured sample.
