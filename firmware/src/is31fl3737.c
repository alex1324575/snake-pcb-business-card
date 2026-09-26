#include "is31fl3737.h"

#define REG_COMMAND 0xfdu
#define REG_UNLOCK 0xfeu
#define COMMAND_KEY 0xc5u
#define PAGE_ENABLE 0u
#define PAGE_PWM 1u
#define PAGE_ABM 2u
#define PAGE_FUNCTION 3u
#define REG_CONFIGURATION 0x00u
#define REG_GLOBAL_CURRENT 0x01u
#define REG_SW_PULLUP 0x0fu
#define REG_CS_PULLDOWN 0x10u

static bool fail_bus(is31fl3737_t *device)
{
    device->io.sdb(false, device->io.user);
    device->ready = false;
    device->last_error = IS31FL3737_ERROR_BUS;
    return false;
}

static bool write_register(is31fl3737_t *device, uint8_t reg, uint8_t value)
{
    const uint8_t bytes[2] = {reg, value};
    if (!device->io.write(IS31FL3737_ADDRESS, bytes, sizeof bytes,
                          device->io.user)) {
        return fail_bus(device);
    }
    return true;
}

static bool select_page(is31fl3737_t *device, uint8_t page)
{
    /* Every FD write consumes the unlock. Do not cache a page across calls:
     * each transaction group restores its own page, including after a fault. */
    return write_register(device, REG_UNLOCK, COMMAND_KEY) &&
           write_register(device, REG_COMMAND, page);
}

static bool write_block(is31fl3737_t *device, uint8_t start,
                        const uint8_t *values, size_t count)
{
    uint8_t bytes[25];
    size_t index;
    bytes[0] = start;
    for (index = 0; index < count; ++index) {
        bytes[index + 1u] = values[index];
    }
    if (!device->io.write(IS31FL3737_ADDRESS, bytes, count + 1u,
                          device->io.user)) {
        return fail_bus(device);
    }
    return true;
}

static bool clear_valid_pixel_registers(is31fl3737_t *device, uint8_t page)
{
    const uint8_t zero[6] = {0};
    uint8_t row;
    if (!select_page(device, page)) {
        return false;
    }
    for (row = 0; row < 12u; ++row) {
        /* The silicon has gaps at x6,x7,xE,xF in every row. Only the
         * six-byte valid spans 0..5 and 8..D may be written. */
        uint8_t start = (uint8_t)(row * 16u);
        if (!write_block(device, start, zero, sizeof zero) ||
            !write_block(device, (uint8_t)(start + 8u), zero, sizeof zero)) {
            return false;
        }
    }
    return true;
}

bool is31fl3737_init(is31fl3737_t *device, const is31fl3737_io_t *io)
{
    uint8_t enable[24] = {0};
    uint8_t row;
    if (device == NULL || io == NULL) {
        return false;
    }
    device->ready = false;
    device->last_error = IS31FL3737_ERROR_ARGUMENT;
    device->global_current = 0;
    device->io = *io;
    if (io->write == NULL || io->sdb == NULL || io->delay_ms == NULL) {
        if (io->sdb != NULL) {
            io->sdb(false, io->user);
        }
        return false;
    }

    /* Only Function registers are available while SDB is low. Establish
     * software shutdown and zero current before releasing hardware shutdown. */
    io->sdb(false, io->user);
    io->delay_ms(1u, io->user);
    if (!select_page(device, PAGE_FUNCTION) ||
        !write_register(device, REG_CONFIGURATION, 0u) ||
        !write_register(device, REG_GLOBAL_CURRENT, 0u)) {
        return false;
    }
    io->sdb(true, io->user);
    io->delay_ms(1u, io->user);
    if (!select_page(device, PAGE_ENABLE) ||
        !write_block(device, 0u, enable, sizeof enable) ||
        !clear_valid_pixel_registers(device, PAGE_PWM) ||
        !clear_valid_pixel_registers(device, PAGE_ABM)) {
        return false;
    }

    for (row = 0; row < IS31FL3737_HEIGHT; ++row) {
        /* D7:D6 are reserved. First byte controls CS1..6, second CS7..12. */
        enable[row * 2u] = 0x3fu;
        enable[row * 2u + 1u] = 0x0fu;
    }
    if (!select_page(device, PAGE_ENABLE) ||
        !write_block(device, 0u, enable, sizeof enable) ||
        !select_page(device, PAGE_FUNCTION) ||
        /* Rev G recommends 32 kohm pull resistors for ghost suppression. */
        !write_register(device, REG_SW_PULLUP, 7u) ||
        !write_register(device, REG_CS_PULLDOWN, 7u) ||
        !write_register(device, REG_GLOBAL_CURRENT, IS31FL3737_MAX_GCC) ||
        !write_register(device, REG_CONFIGURATION, 1u)) {
        return false;
    }
    device->global_current = IS31FL3737_MAX_GCC;
    device->ready = true;
    device->last_error = IS31FL3737_ERROR_NONE;
    return true;
}

bool is31fl3737_write_frame(is31fl3737_t *device,
                          const uint8_t pixels[IS31FL3737_PIXELS])
{
    uint8_t row;
    if (device == NULL) {
        return false;
    }
    if (pixels == NULL) {
        device->last_error = IS31FL3737_ERROR_ARGUMENT;
        return false;
    }
    if (!device->ready) {
        device->last_error = IS31FL3737_ERROR_NOT_INITIALIZED;
        return false;
    }
    if (!select_page(device, PAGE_PWM)) {
        return false;
    }
    for (row = 0; row < IS31FL3737_HEIGHT; ++row) {
        uint8_t start = (uint8_t)(row * 16u);
        const uint8_t *source = &pixels[row * IS31FL3737_WIDTH];
        if (!write_block(device, start, source, 6u) ||
            !write_block(device, (uint8_t)(start + 8u), source + 6u, 4u)) {
            return false;
        }
    }
    device->last_error = IS31FL3737_ERROR_NONE;
    return true;
}

bool is31fl3737_set_global_current(is31fl3737_t *device, uint8_t current)
{
    if (device == NULL) {
        return false;
    }
    if (current > IS31FL3737_MAX_GCC) {
        device->last_error = IS31FL3737_ERROR_ARGUMENT;
        return false;
    }
    if (!device->ready) {
        device->last_error = IS31FL3737_ERROR_NOT_INITIALIZED;
        return false;
    }
    if (!select_page(device, PAGE_FUNCTION) ||
        !write_register(device, REG_GLOBAL_CURRENT, current)) {
        return false;
    }
    device->global_current = current;
    device->last_error = IS31FL3737_ERROR_NONE;
    return true;
}

void is31fl3737_shutdown(is31fl3737_t *device)
{
    if (device != NULL) {
        if (device->io.sdb != NULL) {
            device->io.sdb(false, device->io.user);
        }
        device->ready = false;
    }
}
