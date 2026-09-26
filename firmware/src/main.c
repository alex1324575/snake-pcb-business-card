#include "app.h"
#include "board.h"
#include "is31fl3737.h"
#include <string.h>

/* Inspect through SWD; no UART exists on this board. */
volatile struct {
    uint32_t uptime_ms, frames_written, display_errors, display_recoveries;
    uint32_t app_state;
    bool display_online;
} firmware_status;

int main(void)
{
    app_t app;
    is31fl3737_t display;
    const is31fl3737_io_t io = {
        board_i2c_write, board_sdb, board_delay_ms, 0
    };
    uint8_t frame[SNAKE_CAPACITY], previous[SNAKE_CAPACITY];
    uint32_t now, last_render, last_retry;
    bool frame_valid = false;
    bool online;

    board_init();
    board_delay_ms(25u, 0);
    now = board_millis();
    app_init(&app, now, board_seed() ^ now, board_buttons());
    online = is31fl3737_init(&display, &io);
    if (!online) ++firmware_status.display_errors;
    last_render = last_retry = board_millis();
    for (;;) {
        now = board_millis();
        if (online) {
            app_update(&app, now, board_buttons());
            if ((uint32_t)(now - last_render) >= 40u) {
                last_render = now;
                app_render(&app, now, frame);
                if (!frame_valid || memcmp(frame, previous, sizeof(frame)) != 0) {
                    if (is31fl3737_write_frame(&display, frame)) {
                        memcpy(previous, frame, sizeof(frame));
                        frame_valid = true;
                        ++firmware_status.frames_written;
                    } else {
                        online = frame_valid = false;
                        last_retry = board_millis();
                        ++firmware_status.display_errors;
                    }
                }
            }
        } else if ((uint32_t)(now - last_retry) >= 500u) {
            /* An outage freezes the game. On successful reinitialization the
             * user explicitly starts a new game from READY. */
            if (board_i2c_recover() && is31fl3737_init(&display, &io)) {
                now = board_millis();
                app_display_recovered(&app, now, board_buttons());
                online = true;
                frame_valid = false;
                ++firmware_status.display_recoveries;
            } else {
                ++firmware_status.display_errors;
            }
            last_retry = board_millis();
        }
        firmware_status.uptime_ms = board_millis();
        firmware_status.app_state = (uint32_t)app.state;
        firmware_status.display_online = online;
        /* Never feed from an ISR or bus wait: a hung main loop must reset. */
        board_watchdog_feed();
        board_idle();
    }
}

void Default_Handler(void)
{
    volatile uint32_t * const aircr = (volatile uint32_t *)0xe000ed0cu;
    __asm volatile ("cpsid i" ::: "memory");
    board_sdb(false, 0);
    __asm volatile ("dsb" ::: "memory");
    *aircr = 0x05fa0004u;
    __asm volatile ("dsb" ::: "memory");
    for (;;) { __asm volatile ("nop"); }
}
