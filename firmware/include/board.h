#ifndef SNAKE_BOARD_H
#define SNAKE_BOARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Rev B: STM32G030K8T6, HSI16, PA0..4 switches, PA5 SDB, PB6/7 I2C1.
 * All functions except board_millis/board_buttons run in foreground after
 * board_init. The bus callbacks are synchronous; do not call from an ISR. */
void board_init(void);
uint32_t board_millis(void);
/* Raw switches: bits 0..4 = UP, DOWN, LEFT, RIGHT, START; one means pressed. */
uint8_t board_buttons(void);
/* Board identity plus timing noise: a game seed, not a security RNG. */
uint32_t board_seed(void);
void board_sdb(bool high, void *user);
void board_delay_ms(uint32_t milliseconds, void *user);
/* Seven-bit address; buffer includes register byte. Length must be 1..255.
 * Failure blanks SDB and disables I2C. Recover and reinitialize the display
 * before using it again. Timeout is 30 ms for the entire transfer. */
bool board_i2c_write(uint8_t address, const uint8_t *data, size_t length,
                     void *user);
/* Leaves SDB low. Attempts nine open-drain clocks and a STOP; false means
 * a line remains held low. Successful recovery does not restore LED state. */
bool board_i2c_recover(void);
/* Feed from a healthy foreground loop only; never from SysTick. */
void board_watchdog_feed(void);
void board_idle(void);

#endif
