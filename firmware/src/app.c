#include "app.h"
#include "is31fl3737.h"
#include <string.h>

static void enter(app_t *app, app_state_t state, uint32_t now)
{
    app->state = state;
    app->state_since = now;
    app->last_move = now;
}

static void new_game(app_t *app, uint32_t now)
{
    app->seed ^= now + 0x9e3779b9u + app->game.rng_state;
    snake_init(&app->game, app->seed);
    enter(app, APP_PLAYING, now);
}

void app_init(app_t *app, uint32_t now, uint32_t seed, uint8_t boot_buttons)
{
    memset(app, 0, sizeof(*app));
    app->seed = seed;
    snake_init(&app->game, seed);
    buttons_init(&app->buttons, now, boot_buttons);
    app->suppress_start = (boot_buttons & BUTTON_START) != 0u;
    enter(app, app->suppress_start ? APP_DIAGNOSTIC : APP_SPLASH, now);
}

void app_display_recovered(app_t *app, uint32_t now, uint8_t raw_buttons)
{
    buttons_init(&app->buttons, now, raw_buttons);
    app->suppress_start = (raw_buttons & BUTTON_START) != 0u;
    app->start_tracking = app->start_long = false;
    enter(app, APP_READY, now);
}

static void short_start(app_t *app, uint32_t now)
{
    switch (app->state) {
    case APP_SPLASH: case APP_READY: case APP_GAME_OVER: case APP_WON:
        new_game(app, now); break;
    case APP_PLAYING: enter(app, APP_PAUSED, now); break;
    case APP_PAUSED: enter(app, APP_PLAYING, now); break;
    case APP_DIAGNOSTIC: enter(app, APP_READY, now); break;
    default: break;
    }
}

void app_update(app_t *app, uint32_t now, uint8_t raw_buttons)
{
    uint8_t directions;
    buttons_update(&app->buttons, now, raw_buttons);
    if (app->suppress_start) {
        if (!(app->buttons.stable & BUTTON_START)) app->suppress_start = false;
    } else {
        if (app->buttons.pressed & BUTTON_START) {
            app->start_since = now;
            app->start_tracking = true;
            app->start_long = false;
        }
        if (app->start_tracking && !app->start_long &&
            (app->buttons.stable & BUTTON_START) &&
            (uint32_t)(now - app->start_since) >= APP_LONG_PRESS_MS) {
            app->start_long = true;
            enter(app, APP_READY, now);
        }
        if (app->buttons.released & BUTTON_START) {
            if (app->start_tracking && !app->start_long) short_start(app, now);
            app->start_tracking = false;
        }
    }
    if (app->state == APP_SPLASH &&
        (uint32_t)(now - app->state_since) >= APP_SPLASH_MS) {
        enter(app, APP_READY, now);
    }
    if (app->state != APP_PLAYING) return;

    directions = app->buttons.pressed & (BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT);
    /* Simultaneous new directions are ambiguous: ignore the chord. */
    if (directions && !(directions & (uint8_t)(directions - 1u))) {
        switch (directions) {
        case BUTTON_UP: (void)snake_queue_turn(&app->game, SNAKE_UP); break;
        case BUTTON_DOWN: (void)snake_queue_turn(&app->game, SNAKE_DOWN); break;
        case BUTTON_LEFT: (void)snake_queue_turn(&app->game, SNAKE_LEFT); break;
        case BUTTON_RIGHT: (void)snake_queue_turn(&app->game, SNAKE_RIGHT); break;
        default: break;
        }
    }
    if ((uint32_t)(now - app->last_move) >= snake_step_interval_ms(&app->game)) {
        snake_status_t result = snake_step(&app->game);
        /* No catch-up burst after debugger halt or other long delay. */
        app->last_move = now;
        if (app->game.score > app->high_score) app->high_score = app->game.score;
        if (result == SNAKE_GAME_OVER) enter(app, APP_GAME_OVER, now);
        else if (result == SNAKE_WON) enter(app, APP_WON, now);
    }
}

static const uint8_t digits[10][5] = {
    {7,5,5,5,7}, {2,6,2,2,7}, {7,1,7,4,7}, {7,1,7,1,7}, {5,5,7,1,1},
    {7,4,7,1,7}, {7,4,7,5,7}, {7,1,1,1,1}, {7,5,7,5,7}, {7,5,7,1,7}
};
static void glyph(uint8_t *pixels, unsigned x, const uint8_t rows[5], uint8_t pwm)
{
    unsigned y, dx;
    for (y = 0u; y < 5u; ++y)
        for (dx = 0u; dx < 3u; ++dx)
            if (rows[y] & (1u << (2u - dx))) pixels[(y + 2u) * SNAKE_WIDTH + x + dx] = pwm;
}
static void score_frame(uint8_t *pixels, uint8_t score)
{
    glyph(pixels, 1u, digits[score / 10u], IS31FL3737_PWM_HEAD);
    glyph(pixels, 6u, digits[score % 10u], IS31FL3737_PWM_HEAD);
}
static void game_frame(const snake_t *game, uint32_t now, uint8_t *pixels)
{
    unsigned i;
    for (i = 0u; i < game->length; ++i) pixels[game->cells[i]] = IS31FL3737_PWM_BODY;
    pixels[snake_head(game)] = IS31FL3737_PWM_HEAD;
    if (game->food != SNAKE_NO_CELL)
        pixels[game->food] = ((now / 250u) & 1u) ? IS31FL3737_PWM_FOOD : 24u;
}

void app_render(const app_t *app, uint32_t now, uint8_t pixels[SNAKE_CAPACITY])
{
    /* Example identity: Alex Xu. Each row is a three-bit glyph. */
    static const uint8_t letter_a[5] = {2,5,7,5,5};
    static const uint8_t letter_x[5] = {5,5,2,5,5};
    static const uint8_t ready_path[] = {22,23,24,25,26,36,46,45,44,43,53,63,64,65,66,67};
    unsigned i;
    uint32_t elapsed = now - app->state_since;
    memset(pixels, 0, SNAKE_CAPACITY);
    switch (app->state) {
    case APP_SPLASH:
        glyph(pixels, 1u, letter_a, IS31FL3737_PWM_HEAD);
        glyph(pixels, 6u, letter_x, IS31FL3737_PWM_HEAD);
        break;
    case APP_READY:
        if (app->buttons.stable & BUTTON_UP) score_frame(pixels, app->high_score);
        else {
            for (i = 0u; i < sizeof(ready_path); ++i) pixels[ready_path[i]] = IS31FL3737_PWM_BODY;
            pixels[ready_path[(elapsed / 120u) % sizeof(ready_path)]] = IS31FL3737_PWM_FOOD;
        }
        break;
    case APP_PLAYING: game_frame(&app->game, now, pixels); break;
    case APP_PAUSED:
        for (i = 2u; i < 7u; ++i) {
            pixels[i * SNAKE_WIDTH + 3u] = IS31FL3737_PWM_HEAD;
            pixels[i * SNAKE_WIDTH + 6u] = IS31FL3737_PWM_HEAD;
        }
        break;
    case APP_GAME_OVER: case APP_WON:
        if (elapsed >= APP_RESULT_ANIMATION_MS) score_frame(pixels, app->game.score);
        else if (((elapsed / 150u) & 1u) == 0u) game_frame(&app->game, now, pixels);
        break;
    case APP_DIAGNOSTIC:
        pixels[(elapsed / 120u) % SNAKE_CAPACITY] = IS31FL3737_PWM_HEAD;
        if (app->buttons.stable & BUTTON_UP) pixels[4] = IS31FL3737_PWM_FOOD;
        if (app->buttons.stable & BUTTON_DOWN) pixels[84] = IS31FL3737_PWM_FOOD;
        if (app->buttons.stable & BUTTON_LEFT) pixels[40] = IS31FL3737_PWM_FOOD;
        if (app->buttons.stable & BUTTON_RIGHT) pixels[49] = IS31FL3737_PWM_FOOD;
        break;
    default: break;
    }
}
