#ifndef IS31FL3737_H
#define IS31FL3737_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IS31FL3737_ADDRESS 0x50u
#define IS31FL3737_WIDTH 10u
#define IS31FL3737_HEIGHT 9u
#define IS31FL3737_PIXELS (IS31FL3737_WIDTH * IS31FL3737_HEIGHT)
/* R24 = 20 kohm: GCC 32 limits a current source to about 5.25 mA peak. */
#define IS31FL3737_MAX_GCC 32u
#define IS31FL3737_PWM_BODY 80u
#define IS31FL3737_PWM_HEAD 180u
#define IS31FL3737_PWM_FOOD 255u

typedef struct {
    /* Synchronous STOP-terminated transfer; address is unshifted seven-bit.
     * The buffer includes the starting register followed by data. Maximum
     * length is 25 bytes. The callback must impose a finite bus timeout. */
    bool (*write)(uint8_t address, const uint8_t *data, size_t length, void *user);
    void (*sdb)(bool high, void *user);
    void (*delay_ms)(uint32_t milliseconds, void *user);
    void *user;
} is31fl3737_io_t;

typedef enum {
    IS31FL3737_ERROR_NONE = 0,
    IS31FL3737_ERROR_ARGUMENT,
    IS31FL3737_ERROR_NOT_INITIALIZED,
    IS31FL3737_ERROR_BUS
} is31fl3737_error_t;

typedef struct {
    is31fl3737_io_t io;
    bool ready;
    uint8_t global_current;
    is31fl3737_error_t last_error;
} is31fl3737_t;

/* A successful initialization leaves the matrix enabled but black. It is
 * safe to repeat after a bus recovery: hardware shutdown retains registers,
 * so this explicitly restores all used register planes before enabling. */
bool is31fl3737_init(is31fl3737_t *device, const is31fl3737_io_t *io);

/* Pixel origin: top-left front view, x rightwards, y downwards. Data is
 * pixels[y * 10 + x], with arbitrary 0..255 PWM. No hardware frame latch:
 * writes take effect row groups at a time. A failed transfer blanks SDB and
 * requires bus recovery and init before another frame can be accepted. */
bool is31fl3737_write_frame(is31fl3737_t *device,
                          const uint8_t pixels[IS31FL3737_PIXELS]);

/* The board's conservative current ceiling is enforced even if every pixel
 * is white. Returns false without changing hardware for an invalid value. */
bool is31fl3737_set_global_current(is31fl3737_t *device, uint8_t current);

/* Immediate hardware blanking; init is required to resume. */
void is31fl3737_shutdown(is31fl3737_t *device);

#endif
