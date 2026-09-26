#include "is31fl3737.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t registers[4][256];
    uint8_t page;
    bool unlocked;
    bool sdb_high;
    unsigned calls;
    unsigned fail_at;
    unsigned delays;
    unsigned sdb_changes;
    unsigned configuration_enable_writes;
    size_t largest_transfer;
} mock_t;

static bool valid_pixel_address(uint8_t address)
{
    const uint8_t column = (uint8_t)(address & 15u);
    return address <= 0xbdu && (column < 6u || (column >= 8u && column < 14u));
}

static bool mock_write(uint8_t address, const uint8_t *bytes, size_t length,
                       void *user)
{
    mock_t *mock = user;
    size_t index;
    assert(address == 0x50u);
    assert(length >= 2u && length <= 25u);
    assert((size_t)bytes[0] + length - 2u <= 255u);
    ++mock->calls;
    if (length > mock->largest_transfer) {
        mock->largest_transfer = length;
    }
    if (mock->calls == mock->fail_at) {
        return false;
    }
    if (bytes[0] == 0xfeu) {
        assert(length == 2u && bytes[1] == 0xc5u);
        mock->unlocked = true;
        return true;
    }
    if (bytes[0] == 0xfdu) {
        assert(length == 2u && bytes[1] <= 3u && mock->unlocked);
        mock->page = bytes[1];
        mock->unlocked = false;
        return true;
    }
    /* Real hardware retains registers with SDB low, and only allows PG3. */
    assert(mock->sdb_high || mock->page == 3u);
    for (index = 1u; index < length; ++index) {
        const uint8_t reg = (uint8_t)(bytes[0] + index - 1u);
        const uint8_t value = bytes[index];
        if (mock->page == 0u) {
            assert(reg < 24u);
            assert((value & 0xc0u) == 0u);
        } else if (mock->page == 1u || mock->page == 2u) {
            assert(valid_pixel_address(reg));
            if (mock->page == 2u) {
                assert((value & 0xfcu) == 0u);
            }
        } else {
            /* Reset 0x11 is read-triggered; a write there is not a reset. */
            assert(reg == 0u || reg == 1u || reg == 0x0fu || reg == 0x10u);
            if (reg == 0u && value == 1u) {
                assert(mock->sdb_high);
                ++mock->configuration_enable_writes;
            }
        }
        mock->registers[mock->page][reg] = value;
    }
    return true;
}

static void mock_sdb(bool high, void *user)
{
    mock_t *mock = user;
    if (high) {
        /* Releasing hardware shutdown must not expose retained bright data. */
        assert(mock->registers[3][0] == 0u);
        assert(mock->registers[3][1] == 0u);
    }
    mock->sdb_high = high;
    ++mock->sdb_changes;
}

static void mock_delay(uint32_t milliseconds, void *user)
{
    mock_t *mock = user;
    assert(milliseconds >= 1u);
    ++mock->delays;
}

static is31fl3737_io_t make_io(mock_t *mock)
{
    is31fl3737_io_t io;
    memset(mock, 0, sizeof *mock);
    /* Nonzero retained state catches reliance on an SDB pulse resetting RAM. */
    memset(mock->registers, 0xff, sizeof mock->registers);
    mock->page = 2u;
    mock->sdb_high = true;
    io.write = mock_write;
    io.sdb = mock_sdb;
    io.delay_ms = mock_delay;
    io.user = mock;
    return io;
}

static void check_initial_state(const mock_t *mock)
{
    unsigned row;
    unsigned col;
    assert(mock->registers[3][0] == 1u);
    assert(mock->registers[3][1] == 32u);
    assert(mock->registers[3][0x0f] == 7u);
    assert(mock->registers[3][0x10] == 7u);
    assert(mock->configuration_enable_writes == 1u);
    assert(mock->sdb_high);
    assert(mock->largest_transfer == 25u);
    assert(mock->delays == 2u);
    for (row = 0; row < 12u; ++row) {
        assert(mock->registers[0][row * 2u] == (row < 9u ? 0x3fu : 0u));
        assert(mock->registers[0][row * 2u + 1u] == (row < 9u ? 0x0fu : 0u));
        for (col = 0; col < 12u; ++col) {
            const unsigned reg = row * 16u + (col < 6u ? col : col + 2u);
            assert(mock->registers[1][reg] == 0u);
            assert(mock->registers[2][reg] == 0u);
        }
        /* Reserved addresses remain untouched even during initialization. */
        assert(mock->registers[1][row * 16u + 6u] == 0xffu);
        assert(mock->registers[1][row * 16u + 7u] == 0xffu);
        assert(mock->registers[2][row * 16u + 14u] == 0xffu);
        assert(mock->registers[2][row * 16u + 15u] == 0xffu);
    }
}

static unsigned test_initialization(void)
{
    mock_t mock;
    is31fl3737_t device;
    is31fl3737_io_t io = make_io(&mock);
    assert(is31fl3737_init(&device, &io));
    assert(device.ready && device.last_error == IS31FL3737_ERROR_NONE);
    assert(device.global_current == IS31FL3737_MAX_GCC);
    check_initial_state(&mock);
    return mock.calls;
}

static void test_pixel_mapping_and_brightness(void)
{
    mock_t mock;
    is31fl3737_t device;
    is31fl3737_io_t io = make_io(&mock);
    uint8_t frame[90];
    unsigned x;
    unsigned y;
    unsigned calls;
    for (x = 0; x < sizeof frame; ++x) {
        frame[x] = (uint8_t)(x + 1u);
    }
    assert(is31fl3737_init(&device, &io));
    calls = mock.calls;
    assert(is31fl3737_write_frame(&device, frame));
    assert(mock.calls - calls == 20u);
    for (y = 0; y < 12u; ++y) {
        for (x = 0; x < 12u; ++x) {
            const unsigned reg = y * 16u + (x < 6u ? x : x + 2u);
            assert(mock.registers[1][reg] ==
                   ((x < 10u && y < 9u) ? frame[y * 10u + x] : 0u));
        }
    }
    /* Explicit edge examples catch the tempting but incorrect linear map. */
    assert(mock.registers[1][0x05] == 6u);
    assert(mock.registers[1][0x08] == 7u);
    assert(mock.registers[1][0x8b] == 90u);
    assert(is31fl3737_set_global_current(&device, 0u));
    assert(mock.registers[3][1] == 0u && device.global_current == 0u);
    assert(is31fl3737_set_global_current(&device, 32u));
    calls = mock.calls;
    assert(!is31fl3737_set_global_current(&device, 33u));
    assert(device.last_error == IS31FL3737_ERROR_ARGUMENT);
    assert(device.ready && mock.calls == calls && mock.registers[3][1] == 32u);
    /* set_global_current leaves PG3 selected; a subsequent frame must select PG1. */
    memset(frame, 255, sizeof frame);
    assert(is31fl3737_write_frame(&device, frame));
    assert(mock.registers[1][0x8b] == 255u);
    is31fl3737_shutdown(&device);
    assert(!device.ready && !mock.sdb_high);
    calls = mock.calls;
    assert(!is31fl3737_write_frame(&device, frame));
    assert(mock.calls == calls);
    assert(device.last_error == IS31FL3737_ERROR_NOT_INITIALIZED);
    assert(is31fl3737_init(&device, &io));
    assert(device.ready && mock.registers[1][0x8b] == 0u);
}

static void test_fault_at_each_initialization_transfer(unsigned total)
{
    unsigned fault;
    for (fault = 1u; fault <= total; ++fault) {
        mock_t mock;
        is31fl3737_t device;
        is31fl3737_io_t io = make_io(&mock);
        mock.fail_at = fault;
        assert(!is31fl3737_init(&device, &io));
        assert(mock.calls == fault); /* Abort immediately, no unbounded retry. */
        assert(!device.ready && !mock.sdb_high);
        assert(device.last_error == IS31FL3737_ERROR_BUS);
        mock.fail_at = 0;
        assert(is31fl3737_init(&device, &io));
        assert(device.ready && mock.registers[1][0x8b] == 0u);
    }
}

static void test_fault_at_each_frame_transfer(void)
{
    unsigned fault;
    for (fault = 1u; fault <= 20u; ++fault) {
        mock_t mock;
        is31fl3737_t device;
        is31fl3737_io_t io = make_io(&mock);
        uint8_t frame[90];
        unsigned calls;
        memset(frame, 255, sizeof frame);
        assert(is31fl3737_init(&device, &io));
        calls = mock.calls;
        mock.fail_at = calls + fault;
        assert(!is31fl3737_write_frame(&device, frame));
        assert(mock.calls == calls + fault);
        assert(!device.ready && !mock.sdb_high);
        assert(device.last_error == IS31FL3737_ERROR_BUS);
        mock.fail_at = 0u;
        assert(is31fl3737_init(&device, &io));
        assert(mock.registers[1][0x8b] == 0u);
        assert(is31fl3737_write_frame(&device, frame));
        assert(mock.registers[1][0x8b] == 255u);
    }
}

static void test_invalid_arguments(void)
{
    mock_t mock;
    is31fl3737_t device = {0};
    is31fl3737_io_t io = make_io(&mock);
    assert(!is31fl3737_init(NULL, &io));
    assert(!is31fl3737_init(&device, NULL));
    assert(!is31fl3737_write_frame(NULL, NULL));
    assert(!is31fl3737_write_frame(&device, NULL));
    assert(!is31fl3737_set_global_current(NULL, 0u));
    is31fl3737_shutdown(NULL);
    io.write = NULL;
    assert(!is31fl3737_init(&device, &io));
    assert(!mock.sdb_high && !device.ready && mock.calls == 0u);
    assert(device.last_error == IS31FL3737_ERROR_ARGUMENT);
}

static void test_fault_at_each_current_transfer(void)
{
    unsigned fault;
    for (fault = 1u; fault <= 3u; ++fault) {
        mock_t mock;
        is31fl3737_t device;
        is31fl3737_io_t io = make_io(&mock);
        unsigned calls;
        assert(is31fl3737_init(&device, &io));
        calls = mock.calls;
        mock.fail_at = calls + fault;
        assert(!is31fl3737_set_global_current(&device, 16u));
        assert(mock.calls == calls + fault);
        assert(!device.ready && !mock.sdb_high);
        assert(device.last_error == IS31FL3737_ERROR_BUS);
        assert(device.global_current == IS31FL3737_MAX_GCC);
    }
}

int main(void)
{
    const unsigned initialization_transfers = test_initialization();
    test_pixel_mapping_and_brightness();
    test_fault_at_each_initialization_transfer(initialization_transfers);
    test_fault_at_each_frame_transfer();
    test_fault_at_each_current_transfer();
    test_invalid_arguments();
    printf("IS31FL3737: mapping, reserved addresses, retained-state init, current ceiling, "
           "%u init, 20 frame and 3 current fault points passed\n", initialization_transfers);
    return 0;
}
