# Firmware / Rev B board contract audit

**PASS** — XML connectivity, physical front orientation and firmware constants agree.

393 counted pin/value/macro/LED-position checks passed; 5 deliberate bad inputs were rejected.

Reproduce from any directory: `python firmware/tools/check_board_contract.py` (adjust the script path to the project location). Uses Python standard library only.

## Inputs

| Input | SHA-256 |
|---|---|
| `exports/SnakeBusinessCard.net.xml` | `b49f364292bd2034c926dcc9b51e9530b3c481464b8ca5a7992970dfa4a6593a` |
| `SnakeBusinessCard.kicad_pcb` | `1d8f6f1a8ddd70a6b2e235cf3d18a1b565d091940c09105f31943f0235e35585` |
| `firmware/include/buttons.h` | `0e182ca8eb5b3384d9a153734e24341c3997be564666b86e13d4c2b42daf46c2` |
| `firmware/include/snake.h` | `d585858d82c923e193bb6db81b9a64e234a83ffcd28d68fddceca383eb933038` |
| `firmware/include/is31fl3737.h` | `e02bf3f2de072857ea072a174e0f3122fc40f9746cb02bc7aa6d30dc550e5c04` |
| `firmware/include/stm32g030_min.h` | `0c76ac808e46fa81c21ce64cb20360ac2de4813dc0f971b5e1da4a7029fad640` |
| `firmware/src/board.c` | `f5f0e214cf2b58bebd1f43a7f7b62b704cc9e557d101683833b25f3e63ce020d` |

## Control and debug connections

| Exact net name | U1 pad | MCU function | Other endpoint |
|---|---:|---|---|
| BTN_UP | 7 | PA0 | SW1 |
| BTN_DOWN | 8 | PA1 | SW2 |
| BTN_LEFT | 9 | PA2 | SW3 |
| BTN_RIGHT | 10 | PA3 | SW4 |
| BTN_START | 11 | PA4 | SW5 |
| I2C_SCL | 30 | PB6 | U2.35 |
| I2C_SDA | 31 | PB7 | U2.34 |
| LED_EN | 12 | PA5 | U2.38 |
| SWDIO | 24 | PA13 | TP6 |
| SWCLK | 25 | PA14 | TP7 |
| NRST | 6 | NRST | TP8 |

Each button has a 10k pull-up to +3V3 and a switch to GND; pressed is low.
I²C has 2.2k pull-ups. LED_EN has a 100k pull-down. TP4 is +3V3 and TP5 is GND.
U2 ADDR is GND, matching address 0x50; R24 is 20k 1% to ground. The exact R_EXT net is `LED_R_EXT`. `LED_IICRST` connects only U2 IICRST and its 10k pull-down.
Unused CS11–12 / SW10–12 and SYNC/INTB are explicit unconnected pins.

## BSP source guards

Checks actual SDB/SCL/SDA mask macros and GPIO base constants in the BSP, plus these narrow source patterns (intentional refactoring requires reviewing the guard):

- Buttons read PA0..PA4 active-low
- SDB callback drives port A
- SDB is preloaded low before configuration
- I2C PB6/PB7 are open-drain
- I2C PB6/PB7 select AF6

The separate BSP review verifies register semantics and timing. These source guards verify board binding; they do not execute hardware register writes.

## Physical matrix

The checker derives sorted X/Y coordinates directly from the native PCB, then requires all 90 front-side footprints to match the row-major firmware index. Front-view X increases rightward; KiCad Y increases downward. LED pad 1 is cathode → SW row; pad 2 is anode → CS column. PCB pad net names are independently compared with the XML for every checked pin.

X positions (mm): `[143.0, 146.5, 150.0, 153.5, 157.0, 160.5, 164.0, 167.5, 171.0, 174.5]`

Y positions (mm): `[87.0, 90.5, 94.0, 97.5, 101.0, 104.5, 108.0, 111.5, 115.0]`

| LED | x,y | PCB X,Y mm | Anode net | Cathode net | PWM register |
|---|---|---|---|---|---|
| D1 | 0,0 | 143,87 | LED_COL1 | LED_ROW1 | 0x00 |
| D2 | 1,0 | 146.5,87 | LED_COL2 | LED_ROW1 | 0x01 |
| D3 | 2,0 | 150,87 | LED_COL3 | LED_ROW1 | 0x02 |
| D4 | 3,0 | 153.5,87 | LED_COL4 | LED_ROW1 | 0x03 |
| D5 | 4,0 | 157,87 | LED_COL5 | LED_ROW1 | 0x04 |
| D6 | 5,0 | 160.5,87 | LED_COL6 | LED_ROW1 | 0x05 |
| D7 | 6,0 | 164,87 | LED_COL7 | LED_ROW1 | 0x08 |
| D8 | 7,0 | 167.5,87 | LED_COL8 | LED_ROW1 | 0x09 |
| D9 | 8,0 | 171,87 | LED_COL9 | LED_ROW1 | 0x0A |
| D10 | 9,0 | 174.5,87 | LED_COL10 | LED_ROW1 | 0x0B |
| D11 | 0,1 | 143,90.5 | LED_COL1 | LED_ROW2 | 0x10 |
| D12 | 1,1 | 146.5,90.5 | LED_COL2 | LED_ROW2 | 0x11 |
| D13 | 2,1 | 150,90.5 | LED_COL3 | LED_ROW2 | 0x12 |
| D14 | 3,1 | 153.5,90.5 | LED_COL4 | LED_ROW2 | 0x13 |
| D15 | 4,1 | 157,90.5 | LED_COL5 | LED_ROW2 | 0x14 |
| D16 | 5,1 | 160.5,90.5 | LED_COL6 | LED_ROW2 | 0x15 |
| D17 | 6,1 | 164,90.5 | LED_COL7 | LED_ROW2 | 0x18 |
| D18 | 7,1 | 167.5,90.5 | LED_COL8 | LED_ROW2 | 0x19 |
| D19 | 8,1 | 171,90.5 | LED_COL9 | LED_ROW2 | 0x1A |
| D20 | 9,1 | 174.5,90.5 | LED_COL10 | LED_ROW2 | 0x1B |
| D21 | 0,2 | 143,94 | LED_COL1 | LED_ROW3 | 0x20 |
| D22 | 1,2 | 146.5,94 | LED_COL2 | LED_ROW3 | 0x21 |
| D23 | 2,2 | 150,94 | LED_COL3 | LED_ROW3 | 0x22 |
| D24 | 3,2 | 153.5,94 | LED_COL4 | LED_ROW3 | 0x23 |
| D25 | 4,2 | 157,94 | LED_COL5 | LED_ROW3 | 0x24 |
| D26 | 5,2 | 160.5,94 | LED_COL6 | LED_ROW3 | 0x25 |
| D27 | 6,2 | 164,94 | LED_COL7 | LED_ROW3 | 0x28 |
| D28 | 7,2 | 167.5,94 | LED_COL8 | LED_ROW3 | 0x29 |
| D29 | 8,2 | 171,94 | LED_COL9 | LED_ROW3 | 0x2A |
| D30 | 9,2 | 174.5,94 | LED_COL10 | LED_ROW3 | 0x2B |
| D31 | 0,3 | 143,97.5 | LED_COL1 | LED_ROW4 | 0x30 |
| D32 | 1,3 | 146.5,97.5 | LED_COL2 | LED_ROW4 | 0x31 |
| D33 | 2,3 | 150,97.5 | LED_COL3 | LED_ROW4 | 0x32 |
| D34 | 3,3 | 153.5,97.5 | LED_COL4 | LED_ROW4 | 0x33 |
| D35 | 4,3 | 157,97.5 | LED_COL5 | LED_ROW4 | 0x34 |
| D36 | 5,3 | 160.5,97.5 | LED_COL6 | LED_ROW4 | 0x35 |
| D37 | 6,3 | 164,97.5 | LED_COL7 | LED_ROW4 | 0x38 |
| D38 | 7,3 | 167.5,97.5 | LED_COL8 | LED_ROW4 | 0x39 |
| D39 | 8,3 | 171,97.5 | LED_COL9 | LED_ROW4 | 0x3A |
| D40 | 9,3 | 174.5,97.5 | LED_COL10 | LED_ROW4 | 0x3B |
| D41 | 0,4 | 143,101 | LED_COL1 | LED_ROW5 | 0x40 |
| D42 | 1,4 | 146.5,101 | LED_COL2 | LED_ROW5 | 0x41 |
| D43 | 2,4 | 150,101 | LED_COL3 | LED_ROW5 | 0x42 |
| D44 | 3,4 | 153.5,101 | LED_COL4 | LED_ROW5 | 0x43 |
| D45 | 4,4 | 157,101 | LED_COL5 | LED_ROW5 | 0x44 |
| D46 | 5,4 | 160.5,101 | LED_COL6 | LED_ROW5 | 0x45 |
| D47 | 6,4 | 164,101 | LED_COL7 | LED_ROW5 | 0x48 |
| D48 | 7,4 | 167.5,101 | LED_COL8 | LED_ROW5 | 0x49 |
| D49 | 8,4 | 171,101 | LED_COL9 | LED_ROW5 | 0x4A |
| D50 | 9,4 | 174.5,101 | LED_COL10 | LED_ROW5 | 0x4B |
| D51 | 0,5 | 143,104.5 | LED_COL1 | LED_ROW6 | 0x50 |
| D52 | 1,5 | 146.5,104.5 | LED_COL2 | LED_ROW6 | 0x51 |
| D53 | 2,5 | 150,104.5 | LED_COL3 | LED_ROW6 | 0x52 |
| D54 | 3,5 | 153.5,104.5 | LED_COL4 | LED_ROW6 | 0x53 |
| D55 | 4,5 | 157,104.5 | LED_COL5 | LED_ROW6 | 0x54 |
| D56 | 5,5 | 160.5,104.5 | LED_COL6 | LED_ROW6 | 0x55 |
| D57 | 6,5 | 164,104.5 | LED_COL7 | LED_ROW6 | 0x58 |
| D58 | 7,5 | 167.5,104.5 | LED_COL8 | LED_ROW6 | 0x59 |
| D59 | 8,5 | 171,104.5 | LED_COL9 | LED_ROW6 | 0x5A |
| D60 | 9,5 | 174.5,104.5 | LED_COL10 | LED_ROW6 | 0x5B |
| D61 | 0,6 | 143,108 | LED_COL1 | LED_ROW7 | 0x60 |
| D62 | 1,6 | 146.5,108 | LED_COL2 | LED_ROW7 | 0x61 |
| D63 | 2,6 | 150,108 | LED_COL3 | LED_ROW7 | 0x62 |
| D64 | 3,6 | 153.5,108 | LED_COL4 | LED_ROW7 | 0x63 |
| D65 | 4,6 | 157,108 | LED_COL5 | LED_ROW7 | 0x64 |
| D66 | 5,6 | 160.5,108 | LED_COL6 | LED_ROW7 | 0x65 |
| D67 | 6,6 | 164,108 | LED_COL7 | LED_ROW7 | 0x68 |
| D68 | 7,6 | 167.5,108 | LED_COL8 | LED_ROW7 | 0x69 |
| D69 | 8,6 | 171,108 | LED_COL9 | LED_ROW7 | 0x6A |
| D70 | 9,6 | 174.5,108 | LED_COL10 | LED_ROW7 | 0x6B |
| D71 | 0,7 | 143,111.5 | LED_COL1 | LED_ROW8 | 0x70 |
| D72 | 1,7 | 146.5,111.5 | LED_COL2 | LED_ROW8 | 0x71 |
| D73 | 2,7 | 150,111.5 | LED_COL3 | LED_ROW8 | 0x72 |
| D74 | 3,7 | 153.5,111.5 | LED_COL4 | LED_ROW8 | 0x73 |
| D75 | 4,7 | 157,111.5 | LED_COL5 | LED_ROW8 | 0x74 |
| D76 | 5,7 | 160.5,111.5 | LED_COL6 | LED_ROW8 | 0x75 |
| D77 | 6,7 | 164,111.5 | LED_COL7 | LED_ROW8 | 0x78 |
| D78 | 7,7 | 167.5,111.5 | LED_COL8 | LED_ROW8 | 0x79 |
| D79 | 8,7 | 171,111.5 | LED_COL9 | LED_ROW8 | 0x7A |
| D80 | 9,7 | 174.5,111.5 | LED_COL10 | LED_ROW8 | 0x7B |
| D81 | 0,8 | 143,115 | LED_COL1 | LED_ROW9 | 0x80 |
| D82 | 1,8 | 146.5,115 | LED_COL2 | LED_ROW9 | 0x81 |
| D83 | 2,8 | 150,115 | LED_COL3 | LED_ROW9 | 0x82 |
| D84 | 3,8 | 153.5,115 | LED_COL4 | LED_ROW9 | 0x83 |
| D85 | 4,8 | 157,115 | LED_COL5 | LED_ROW9 | 0x84 |
| D86 | 5,8 | 160.5,115 | LED_COL6 | LED_ROW9 | 0x85 |
| D87 | 6,8 | 164,115 | LED_COL7 | LED_ROW9 | 0x88 |
| D88 | 7,8 | 167.5,115 | LED_COL8 | LED_ROW9 | 0x89 |
| D89 | 8,8 | 171,115 | LED_COL9 | LED_ROW9 | 0x8A |
| D90 | 9,8 | 174.5,115 | LED_COL10 | LED_ROW9 | 0x8B |

## Negative controls

- LED anode on wrong column
- Front corner LEDs physically swapped
- Firmware UP assigned DOWN bit
- Current resistor changed
- Firmware GCC ceiling removed

## Scope

Checks the exported design and firmware source constants. It does not measure the assembled board, prove copper connectivity (see KiCad DRC), validate a C preprocessor configuration, or replace functional host tests and hardware bring-up. The numerical PWM register column follows the reviewed IS31FL3737 map; the driver mock-bus tests independently exercise actual generated writes.
