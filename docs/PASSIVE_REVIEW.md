# Passive component selection and review

Review date: **2026-09-27**. This review freezes procurement part numbers for the 23 fitted resistors and capacitors. It is a document review, not an assembled-board qualification. C30 remains unpopulated.

## Selected parts

All fitted passives use the existing 0603 / 1608 metric footprints. Nominal capacitances and resistances are unchanged. Capacitor voltage ratings are increased; all resistors are explicitly specified at 1% tolerance.

| References | Qty | Manufacturer part number | Selected specification | Review result |
|---|---:|---|---|---|
| C1, C2, C23, C24 | 4 | [TDK C1608X7R1V105K080AC](https://product.tdk.com/en/search/capacitor/ceramic/mlcc/info?part_no=C1608X7R1V105K080AC) | 1 uF, 35 V, X7R, +/-10% | Rating/package reviewed; effective-capacitance sign-off open |
| C10, C12, C20-C22 | 5 | [TDK C1608X7R1H104K080AA](https://product.tdk.com/en/search/capacitor/ceramic/mlcc/info?part_no=C1608X7R1H104K080AA) | 100 nF, 50 V, X7R, +/-10% | Rating/package reviewed; biased decoupling and reset behavior require board test |
| C11 | 1 | [Murata GRM188Z71C475KE21D](https://www.murata.com/en-us/products/productdetail?partno=GRM188Z71C475KE21D) | 4.7 uF, 16 V, X7R(Murata), +/-10% | Rating/package reviewed; bias characterization open |
| R1, R2 | 2 | [Panasonic ERJ3EKF5101V](https://industrial.panasonic.com/ww/products/pt/general-purpose-chip-resistors/models/ERJ3EKF5101V) | 5.1 kohm, +/-1%, 0.1 W | Nominal value and electrical loading reviewed |
| R10-R15, R23 | 7 | [Panasonic ERJ3EKF1002V](https://industrial.panasonic.com/ww/products/pt/general-purpose-chip-resistors/models/ERJ3EKF1002V) | 10 kohm, +/-1%, 0.1 W | Nominal value and electrical loading reviewed |
| R20, R21 | 2 | [Panasonic ERJ3EKF2201V](https://industrial.panasonic.com/ww/products/pt/general-purpose-chip-resistors/models/ERJ3EKF2201V) | 2.2 kohm, +/-1%, 0.1 W | Electrical loading reviewed; I2C edges require measurement |
| R22 | 1 | [Panasonic ERJ3EKF1003V](https://industrial.panasonic.com/ww/products/pt/general-purpose-chip-resistors/models/ERJ3EKF1003V) | 100 kohm, +/-1%, 0.1 W | Nominal value and electrical loading reviewed |
| R24 | 1 | [Panasonic ERJ3EKF2002V](https://industrial.panasonic.com/ww/products/pt/general-purpose-chip-resistors/models/ERJ3EKF2002V) | 20 kohm, +/-1%, 0.1 W | Current-setting value and electrical loading reviewed |
| C30 | 0 fitted | **No part number: DNP** | Reserved C0G tuning position | Select capacitance only after measuring the assembled NFC network |

The suffixes above are complete manufacturer catalog identifiers. The TDK parts are supplied on 180 mm paper-tape reels; the Murata `D` suffix selects its 4,000-piece, 180 mm paper-tape reel. Panasonic `V` selects paper carrier tape. Distributor cut tape can use the same manufacturer part number; distributor stock codes are not substitutes for the MPN. See the [Murata reference sheet](https://search.murata.co.jp/Ceramy/image/img/A01X/G101/ENG/GRM188Z71C475KE21-01A.pdf) and [Panasonic ordering information](https://industrial.panasonic.com/cdbs/www-data/pdf/RDA0000/AOA0000C304.pdf).

## Capacitor checks and the remaining release condition

| Position | Applied DC voltage used for review | Rating utilization | Function |
|---|---:|---:|---|
| C1 | 5 V nominal; check capacitance at 5.5 V as the input design ceiling | 15.7% of 35 V at the ceiling | Local regulator input capacitor |
| C2 | 3.3 V nominal; check at the regulator's maximum specified output | 9.4% of 35 V at nominal | Local regulator output capacitor |
| C23, C24 | 3.3 V nominal | 9.4% of 35 V | LED-driver bulk bypass |
| C10, C12, C20-C22 | Up to 3.3 V nominal | 6.6% of 50 V | MCU/driver bypass and reset filtering |
| C11 | 3.3 V nominal | 20.6% of 16 V | MCU supply bulk bypass |

These percentages check voltage stress only. They do **not** establish effective capacitance. The [TLV755P datasheet, pin-function note and section 7.1.1](https://www.ti.com/lit/ds/symlink/tlv755p.pdf) require local input/output capacitors with at least 1 uF nominal and more than 0.47 uF effective capacitance at the pins. Remote capacitors elsewhere on the 3.3 V net are not used to waive that local requirement.

For a 1 uF part, a screening calculation using -10% initial tolerance and -15% temperature variation leaves `1.0 x 0.90 x 0.85 = 0.765 uF` **before** DC bias and aging. The remaining combined retention factor must exceed `0.47 / 0.765 = 0.6144`. This is an acceptance budget, not a measured result or a guaranteed combined-temperature model. Voltage rating alone does not prove that the budget is met.

TDK supplies a [1 uF characterization sheet](https://product.tdk.com/system/files/dam/doc/product/capacitor/ceramic/mlcc/charasheet/c1608x7r1v105k080ac.pdf) and a [100 nF characterization sheet](https://product.tdk.com/info/en/documents/chara_sheet/C1608X7R1H104K080AA.pdf). The product records identify DC-bias data, but numerical capacitance at the board's bias voltages was not obtained in this review. Consequently **C1/C2 are conditionally selected, not signed off as meeting the effective-capacitance limit**. Before production release, capture the exact-part bias/temperature data, include aging and tolerance, and demonstrate the local minimum above 0.47 uF. Confirm startup, ripple and load-step response on an assembled board.

C11's `X7R(Murata)` designation is intentionally retained in the purchasing specification. Its temperature test is defined with half of rated voltage applied, and must not be silently treated as the ordinary EIA X7R test. Its effective capacitance at 3.3 V remains to be recorded from exact-part characterization. The nominal bulk value is unchanged.

## Resistor margin

The selected ERJ3EK parts are rated 0.1 W at 70 C with a 75 V limiting element voltage. Actual continuous allowable voltage is the lower of that limit and `sqrt(P_rating x R)`; power must be derated above 70 C. The family data gives +/-100 ppm/K temperature coefficient for these values. These ratings come from the [Panasonic family datasheet](https://industrial.panasonic.com/cdbs/www-data/pdf/RDA0000/AOA0000C304.pdf).

The following are calculated DC screening bounds, not measured waveforms. Values use the -1% resistance corner and conservative applied voltages. A 5.5 V bound on the CC resistors is higher than their normal operating voltage; it is not an ESD/surge qualification.

| References | Assumed voltage across resistor | Calculated power | Fraction of 0.1 W |
|---|---:|---:|---:|
| R1, R2 | 5.5 V | 5.99 mW | 6.0% |
| R10-R15, R23 | 3.6 V | 1.31 mW | 1.3% |
| R20, R21 | 3.6 V | 5.95 mW | 6.0% |
| R22 | 3.6 V | 0.131 mW | 0.13% |
| R24 | 3.6 V, deliberately above the driver's reference voltage | 0.655 mW | 0.65% |

The lowest continuous-voltage bound is about 14.8 V for a 2.2 kohm resistor at -1%, above its 3.6 V screening voltage. Initial resistance ranges are 5.049-5.151 kohm for R1/R2 and 19.8-20.2 kohm for R24; temperature and aging remain additional terms. R24 tolerance alone changes an inverse-resistance LED-current calculation by approximately -0.99%/+1.01%; driver tolerances must also be included. I2C rise time, reset timing and LED current still require assembled-board measurements.

## Footprint and assembly review

The PCB uses KiCad's nominal IPC 0603 lands, with two nonpolar terminals per part. Capacitor pads are 0.90 x 0.95 mm at a 1.55 mm center spacing, leaving a 0.65 mm gap. Resistor pads are 0.80 x 0.95 mm at a 1.65 mm center spacing, leaving a 0.85 mm gap. These are native footprint dimensions, not inferred from the rendered pictures.

The selected parts have 1.6 x 0.8 mm nominal bodies. The existing lands are **not exact copies of every manufacturer recommendation**: TDK's C1608 reflow recommendation uses 0.6-0.8 mm dimensions, and the existing pads are wider. Keep this deviation visible to the assembler and approve stencil apertures, solder volume and the reflow profile before a production run. No pad or routing change is made by this selection. See [TDK mounting guidance](https://product.tdk.com/system/files/dam/doc/product/capacitor/ceramic/mlcc/specification/mlccspec_commercial_general_midvoltage_en.pdf), [Murata mounting guidance](https://search.murata.co.jp/Ceramy/image/img/A01X/G101/ENG/GRM188Z71C475KE21-01A.pdf), and [Panasonic land-pattern guidance](https://industrial.panasonic.com/cdbs/www-data/pdf/RDM0000/DMM0000COL17.pdf).

## Procurement status and closure checklist

TDK's current product records and [Meister part list](https://product.tdk.com/system/files/dam/technicalsupport/tdk_meister/tdk_meister_partslist_en.pdf) show both selected TDK parts in production. Manufacturer records exist for the exact Murata and Panasonic suffixes; no independent lifetime-supply or stock guarantee is inferred. Distributor quantities, prices and lead times must be checked at purchase. No components have been ordered.

- [x] Exact MPN, nominal value, tolerance and 0603 package assigned to every fitted passive.
- [x] Capacitor voltage stress and resistor DC power calculated.
- [x] C30 excluded from purchase and assembly.
- [ ] Exact-part effective capacitance established at 5 V/5.5 V and 3.3 V, including tolerance, temperature and aging; C1/C2 minimum above 0.47 uF.
- [ ] Assembler acceptance of existing lands and paste process recorded.
- [ ] Power transient, reset, I2C and LED-current checks completed on the first board.
- [ ] Supplier availability and authorized sourcing confirmed for the actual order.

Machine-readable selections are in [passive_selection.json](../verification/hardware/passive_selection.json).
