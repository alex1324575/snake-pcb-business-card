#ifndef APP_H
#define APP_H
#include "buttons.h"
#include "snake.h"
#include <stdbool.h>
#include <stdint.h>

#define APP_SPLASH_MS 800u
#define APP_LONG_PRESS_MS 800u
#define APP_RESULT_ANIMATION_MS 900u
typedef enum {
    APP_SPLASH, APP_READY, APP_PLAYING, APP_PAUSED,
    APP_GAME_OVER, APP_WON, APP_DIAGNOSTIC
} app_state_t;
typedef struct {
    snake_t game;
    buttons_t buttons;
    app_state_t state;
    uint32_t state_since, last_move, start_since, seed;
    uint8_t high_score;
    bool start_tracking, start_long, suppress_start;
} app_t;

/* START held at boot selects the LED/button diagnostic. The boot release
 * is consumed; a fresh short START press exits it. No flash writes. */
void app_init(app_t *app, uint32_t now, uint32_t seed, uint8_t boot_buttons);
void app_update(app_t *app, uint32_t now, uint8_t raw_buttons);
void app_render(const app_t *app, uint32_t now, uint8_t pixels[SNAKE_CAPACITY]);
/* After a display outage, return to READY instead of advancing invisibly. */
void app_display_recovered(app_t *app, uint32_t now, uint8_t raw_buttons);
#endif
