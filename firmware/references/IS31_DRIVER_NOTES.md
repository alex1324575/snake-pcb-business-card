# IS31FL3737 driver for Snake Card Rev B

## Source and board binding

Checked against the retained `../../references/IS31FL3737-RevG.pdf` and the
[manufacturer's Rev G datasheet](https://www.lumissil.com/assets/pdf/core/IS31FL3737_DS.pdf)
on 2026-09-17. Relevant sections are Table 1 (p.9), Tables 2–3 (pp.11–12),
Tables 6–7 (p.14), Figure 9 (p.15), Tables 13–14 (pp.17–18), and shutdown
discussion (p.24). Figure 9 was visually inspected because text extraction
does not contain its address matrix.

The board netlist assigns U2 ADDR to GND, CS1–CS10 to the ten LED anode columns,
SW1–SW9 to the nine cathode rows, and R24 to 20 kΩ. The driver uses the
unshifted seven-bit I²C address `0x50`. The transport adds the write bit.

Register page selection requires `FE=C5` followed by `FD=page`; an FD write
consumes the unlock. The PWM address for a zero-based `(x,y)` is
`16*y + (x < 6 ? x : x+2)`. Example: `(6,0)` is `08`, `(9,8)` is `8B`.
Each row has gaps at offsets 6, 7, E and F. Enable bytes use six bits each.
Hardware shutdown retains registers and permits access to Function registers.
Register PG3:11 resets on **read**, so the write-only transport does not use it.

## Implemented behavior

- The public frame is 90 bytes, row-major, front-view origin at upper left.
- Each page selection unlocks anew. There is no cached page state.
- Startup drives SDB low, writes software shutdown and zero global current,
  then raises SDB while software shutdown remains active.
- It clears all 24 enable bytes, all 144 valid PWM bytes and all 144 valid
  auto-breath selection bytes. It never writes the reserved PWM address gaps.
- It then enables `3F,0F` for rows 1–9 and `00,00` for rows 10–12. CS11–CS12
  and SW10–SW12 remain disabled even when the caller draws a full white frame.
- It selects the 32 kΩ deghost settings, GCC=32, and enables PWM mode last.
  A successful initialization therefore shows a black display.
- A frame takes 20 synchronous transfers: page unlock/select, then 6-byte and
  4-byte spans for each of nine rows. Initialization takes 68 transfers.
- There is no global hardware frame latch. The application finishes drawing
  its software frame before submitting it; row groups update during transfer.
- Any NACK/timeout reported by the transport immediately lowers SDB, clears
  `ready`, returns false, and stores `IS31FL3737_ERROR_BUS`. The driver does
  not retry. The application must recover the bus, initialize the driver,
  then submit its current complete frame again.
- `shutdown` blanks with SDB and requires initialization to resume.
- The longest callback buffer is 25 bytes, including its register byte.
  Callbacks must finish before returning and impose finite bus timeouts.

## Brightness and current budget

The driver enforces GCC ≤32. With the fitted 20 kΩ resistor, the nominal
formula gives about 5.25 mA per active source and 0.410 mA time-averaged per
pixel at PWM=255. Ninety pixels at full PWM are therefore approximately
36.9 mA of LED current. Driver quiescent current, MCU current, resistor and IC
tolerances must be added; this is a calculated estimate, not a measurement.

Named starting levels are body=80, head=180, food=255. The application can
animate these within the 0–255 PWM range, for example pulsing the food so it
remains distinguishable on the monochrome display. Changing PWM never bypasses
the global current ceiling. Actual visibility and ghosting need checking on
the assembled board under expected room lighting.

## Verification

`tests/test_is31fl3737.c` models the chip register planes and rejects illegal
page access, missing unlocks, reserved addresses, reserved enable bits, and
unsafe release of SDB. It starts with nonzero retained state and checks every
pixel mapping and unused row/column. It injects failures at every one of the
68 initialization and 20 frame transfers, checks immediate blanking and
absence of retries, then verifies successful reinitialization and redraw.

The test also checks all three GCC-update transfer failure points, the GCC
ceiling and zero setting, reselecting PWM after a GCC update, explicit shutdown,
and invalid arguments. It passed on Windows x64 with MSVC `/std:c11 /W4 /WX`
on 2026-09-17; output is in `../build/host/is31fl3737-test.log`. Host tests validate
software contracts and register traffic; they cannot prove I²C signal quality,
physical LED polarity, assembled orientation, brightness or real bus recovery.
