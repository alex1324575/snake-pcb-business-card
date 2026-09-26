# Hardware

Snake PCB Business Card combines a USB-powered game with an independent passive NFC contact tag. The editable design is a **prototype with digital verification; an assembled board has not been tested**. See [verification scope](VERIFICATION.md).

Open [SnakeBusinessCard.kicad_pro](../SnakeBusinessCard.kicad_pro) in KiCad. The root schematic contains five functional sheets. A [schematic PDF](../exports/SnakeBusinessCard_Schematic.pdf) is available for review without KiCad.

![Front of the example board](images/front.png)

## Architecture

```mermaid
flowchart LR
    USB[USB-C 5 V] --> Protection[Input and CC protection]
    Protection --> LDO[TLV75533: 3.3 V]
    LDO --> MCU[STM32G030K8T6]
    LDO --> Driver[IS31FL3737]
    Buttons[Four directions + START] --> MCU
    MCU -->|I2C and enable| Driver
    Driver --> Matrix[10 × 9 red LEDs]
    SWD[Rear SWD pads] --- MCU
    Antenna[Printed NFC antenna] --- Tag[ST25TN01K]
    Tag -. passive phone read .-> URL[https://example.com/]
    QR[Rear silkscreen QR] -. camera scan .-> URL
```

The USB connector supplies power only. It has no USB data connection or firmware upload path. There is no battery. NFC operates independently of the MCU and USB supply; game firmware cannot write this tag through the current circuit.

## Design choices

| Item | Design |
|---|---|
| Outline | 85.6 × 54 mm, 2 mm corner radius |
| PCB | Two copper layers, 0.8 mm board thickness |
| Finish | Graphite black solder mask, white silkscreen, ENIG |
| Controller | STM32G030K8T6, LQFP32, internal 16 MHz clock |
| Display | IS31FL3737 and 90 APT1608SURCK red LEDs, 3.5 mm pitch |
| Input | Five SKQGABE010 momentary switches |
| Regulator | TLV75533PDBVR, 3.3 V |
| NFC | ST25TN01K-AFH5 and an eight-turn printed antenna |
| Example identity | ALEX PARKER / Electrical Engineer |

The 0.8 mm dimension describes the PCB substrate; components and the connector add height. The simplified USB/NFC 3D models show placement envelopes, not complete manufacturer mechanical tolerances. Procurement and assembly details are in [Assembly](ASSEMBLY.md).

## MCU connections

| Function | U1 physical pin | GPIO | Exact net |
|---|---:|---|---|
| Up | 7 | PA0 | BTN_UP |
| Down | 8 | PA1 | BTN_DOWN |
| Left | 9 | PA2 | BTN_LEFT |
| Right | 10 | PA3 | BTN_RIGHT |
| Start | 11 | PA4 | BTN_START |
| Display enable | 12 | PA5 | LED_EN |
| I²C clock | 30 | PB6, AF6 | I2C_SCL |
| I²C data | 31 | PB7, AF6 | I2C_SDA |
| SWD data | 24 | PA13 | SWDIO |
| SWD clock | 25 | PA14 | SWCLK |
| Reset | 6 | NRST | NRST |

Buttons connect to ground when pressed and have external 10 kΩ pull-ups. I²C has 2.2 kΩ pull-ups to 3.3 V. LED_EN has a 100 kΩ pull-down so the display starts disabled. SWD remains available during normal firmware operation. See the [STM32G030 datasheet](https://www.st.com/resource/en/datasheet/stm32g030k8.pdf) for the device pinout.

## LED mapping and current

Front-view coordinates start at the upper-left LED: `D(1 + x + 10*y)`, with `x=0..9`, `y=0..8`. Thus D1/D10/D81/D90 are the four corners. Each LED's pad 2 is its anode, connected to `LED_COL1..10` / CS1..CS10; pad 1 is its cathode, connected to `LED_ROW1..9` / SW1..SW9.

The driver address is seven-bit `0x50`. R24 is 20 kΩ, and firmware limits global current to GCC=32. This gives about 5.25 mA nominal source current while a channel is active and approximately 36.9 mA combined average LED current with all 90 pixels at full PWM. MCU/driver current and component tolerances are additional; these are calculations, not measurements. The driver's sparse PWM address map and initialization sequence are documented in [IS31 driver notes](../firmware/references/IS31_DRIVER_NOTES.md) and the [manufacturer datasheet](https://www.lumissil.com/assets/pdf/core/IS31FL3737_DS.pdf).

## NFC layout boundary

The antenna and feed area exclude unrelated copper on both layers. QR1 is rear silkscreen only, with no metal or mask openings. C30 is an unpopulated tuning position. The antenna geometry has not been tuned or tested with phones; see [Contact and NFC](CONTACT.md).
