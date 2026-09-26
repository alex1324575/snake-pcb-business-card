#include "app.h"
#include "is31fl3737.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned lit_pixels(const uint8_t pixels[SNAKE_CAPACITY])
{
    unsigned count = 0u;
    unsigned i;
    for (i = 0u; i < SNAKE_CAPACITY; ++i) {
        if (pixels[i] != 0u) ++count;
    }
    return count;
}

static uint32_t short_start(app_t *app, uint32_t when)
{
    app_update(app, when, BUTTON_START);
    app_update(app, when + 19u, BUTTON_START);
    app_update(app, when + 20u, BUTTON_START);
    app_update(app, when + 60u, 0u);
    app_update(app, when + 79u, 0u);
    app_update(app, when + 80u, 0u);
    return when + 80u;
}

static void init_playing(app_t *app, uint32_t now)
{
    app_init(app, now, 123u, 0u);
    app->state = APP_PLAYING;
    app->game.food = 0u;
}

static void test_per_key_debounce(void)
{
    buttons_t b;
    buttons_init(&b, 0u, 0u);
    buttons_update(&b, 10u, BUTTON_UP);
    assert(b.stable == 0u && b.pressed == 0u);
    buttons_update(&b, 15u, BUTTON_UP | BUTTON_DOWN);
    buttons_update(&b, 18u, BUTTON_DOWN);
    buttons_update(&b, 20u, BUTTON_UP | BUTTON_DOWN);
    buttons_update(&b, 35u, BUTTON_UP | BUTTON_DOWN);
    assert(b.stable == BUTTON_DOWN && b.pressed == BUTTON_DOWN);
    buttons_update(&b, 39u, BUTTON_UP | BUTTON_DOWN);
    assert(b.stable == BUTTON_DOWN && b.pressed == 0u);
    buttons_update(&b, 40u, BUTTON_UP | BUTTON_DOWN);
    assert(b.stable == (BUTTON_UP | BUTTON_DOWN));
    assert(b.pressed == BUTTON_UP && b.released == 0u);
    buttons_update(&b, 41u, BUTTON_UP | BUTTON_DOWN);
    assert(b.pressed == 0u && b.released == 0u);
    buttons_update(&b, 50u, BUTTON_DOWN);
    buttons_update(&b, 55u, BUTTON_UP | BUTTON_DOWN);
    buttons_update(&b, 60u, BUTTON_DOWN);
    buttons_update(&b, 79u, BUTTON_DOWN);
    assert(b.stable == (BUTTON_UP | BUTTON_DOWN) && b.released == 0u);
    buttons_update(&b, 80u, BUTTON_DOWN);
    assert(b.stable == BUTTON_DOWN && b.released == BUTTON_UP);
    buttons_update(&b, 81u, BUTTON_DOWN);
    assert(b.pressed == 0u && b.released == 0u);
    buttons_init(&b, 100u, 0x80u);
    assert(b.stable == 0u);
    buttons_update(&b, 110u, 0xffu);
    buttons_update(&b, 130u, 0xffu);
    assert(b.stable == BUTTON_MASK && b.pressed == BUTTON_MASK);
}

static void test_uptime_wrap(void)
{
    buttons_t b;
    app_t app;
    const uint32_t begin = UINT32_MAX - 10u;
    buttons_init(&b, begin - 1u, 0u);
    buttons_update(&b, begin, BUTTON_LEFT);
    buttons_update(&b, begin + 19u, BUTTON_LEFT);
    assert(b.stable == 0u);
    buttons_update(&b, begin + 20u, BUTTON_LEFT);
    assert(b.stable == BUTTON_LEFT && b.pressed == BUTTON_LEFT);

    app_init(&app, begin, 0u, 0u);
    app_update(&app, begin + APP_SPLASH_MS - 1u, 0u);
    assert(app.state == APP_SPLASH);
    app_update(&app, begin + APP_SPLASH_MS, 0u);
    assert(app.state == APP_READY);

    init_playing(&app, begin);
    app_update(&app, begin + 319u, 0u);
    assert(snake_head(&app.game) == 45u);
    app_update(&app, begin + 320u, 0u);
    assert(snake_head(&app.game) == 46u);

    app_init(&app, begin - 100u, 0u, 0u);
    app_update(&app, begin - 20u, BUTTON_START);
    app_update(&app, begin, BUTTON_START);
    app_update(&app, begin + APP_LONG_PRESS_MS - 1u, BUTTON_START);
    assert(!app.start_long);
    app_update(&app, begin + APP_LONG_PRESS_MS, BUTTON_START);
    assert(app.start_long && app.state == APP_READY);
}

static void test_boot_diagnostic_release(void)
{
    app_t app;
    uint8_t pixels[SNAKE_CAPACITY];
    app_init(&app, 0u, 42u, BUTTON_START);
    assert(app.state == APP_DIAGNOSTIC && app.suppress_start);
    app_update(&app, 1500u, BUTTON_START);
    assert(app.state == APP_DIAGNOSTIC && !app.start_tracking);
    app_render(&app, 120u, pixels);
    assert(pixels[1] == IS31FL3737_PWM_HEAD && lit_pixels(pixels) == 1u);
    app_update(&app, 1600u, 0u);
    app_update(&app, 1619u, 0u);
    assert(app.suppress_start);
    app_update(&app, 1620u, 0u);
    assert(!app.suppress_start && app.state == APP_DIAGNOSTIC);
    app_update(&app, 1700u, BUTTON_UP | BUTTON_RIGHT);
    app_update(&app, 1720u, BUTTON_UP | BUTTON_RIGHT);
    app_render(&app, 1720u, pixels);
    assert(pixels[4] == IS31FL3737_PWM_FOOD);
    assert(pixels[49] == IS31FL3737_PWM_FOOD);
    app_update(&app, 1750u, 0u);
    app_update(&app, 1770u, 0u);
    (void)short_start(&app, 1800u);
    assert(app.state == APP_READY);
}

static void test_short_start_pause_resume(void)
{
    app_t app;
    uint8_t pixels[SNAKE_CAPACITY];
    uint8_t cells[SNAKE_CAPACITY];
    app_init(&app, 0u, 99u, 0u);
    app_update(&app, APP_SPLASH_MS, 0u);
    assert(app.state == APP_READY);
    (void)short_start(&app, 900u);
    assert(app.state == APP_PLAYING && app.last_move == 980u);
    app.game.food = 0u;
    memcpy(cells, app.game.cells, sizeof(cells));
    (void)short_start(&app, 1000u);
    assert(app.state == APP_PAUSED);
    app_render(&app, 1100u, pixels);
    assert(lit_pixels(pixels) == 10u);
    assert(pixels[23] == IS31FL3737_PWM_HEAD && pixels[66] == IS31FL3737_PWM_HEAD);
    app_update(&app, 10000u, 0u);
    assert(memcmp(cells, app.game.cells, sizeof(cells)) == 0);
    (void)short_start(&app, 10100u);
    assert(app.state == APP_PLAYING && app.last_move == 10180u);
    app_update(&app, 10499u, 0u);
    assert(snake_head(&app.game) == 45u);
    app_update(&app, 10500u, 0u);
    assert(snake_head(&app.game) == 46u);
}

static void test_long_start_release_guard(void)
{
    app_t app;
    app_init(&app, 0u, 1u, 0u);
    app.state = APP_PAUSED;
    app.high_score = 8u;
    app_update(&app, 100u, BUTTON_START);
    app_update(&app, 120u, BUTTON_START);
    app_update(&app, 919u, BUTTON_START);
    assert(app.state == APP_PAUSED && !app.start_long);
    app_update(&app, 920u, BUTTON_START);
    assert(app.state == APP_READY && app.start_long);
    app_update(&app, 5000u, BUTTON_START);
    assert(app.state == APP_READY && app.state_since == 920u);
    app_update(&app, 5100u, 0u);
    app_update(&app, 5120u, 0u);
    assert(app.state == APP_READY && !app.start_tracking);
    assert(app.high_score == 8u);
    (void)short_start(&app, 5200u);
    assert(app.state == APP_PLAYING && app.high_score == 8u);
}

static void assert_score_twelve(const uint8_t pixels[SNAKE_CAPACITY])
{
    static const char expected[9][11] = {
        "          ", "          ", "  #   ### ",
        " ##     # ", "  #   ### ", "  #   #   ",
        " ###  ### ", "          ", "          "
    };
    unsigned x;
    unsigned y;
    for (y = 0u; y < SNAKE_HEIGHT; ++y) {
        for (x = 0u; x < SNAKE_WIDTH; ++x) {
            assert(pixels[y * SNAKE_WIDTH + x] ==
                   (expected[y][x] == '#' ? IS31FL3737_PWM_HEAD : 0u));
        }
    }
}

static void test_wall_result_score_and_high_score(void)
{
    static const uint8_t body[] = {
        49u, 48u, 47u, 46u, 45u, 44u, 43u, 42u, 41u, 40u,
        30u, 31u, 32u, 33u, 34u
    };
    app_t app;
    uint8_t pixels[SNAKE_CAPACITY];
    init_playing(&app, 0u);
    memcpy(app.game.cells, body, sizeof(body));
    app.game.length = (uint8_t)sizeof(body);
    app.game.score = 12u;
    app.game.food = 0u;
    app_update(&app, 200u, 0u);
    assert(app.state == APP_GAME_OVER && app.game.status == SNAKE_GAME_OVER);
    assert(app.high_score == 12u && app.state_since == 200u);
    app_render(&app, 200u, pixels);
    assert(pixels[49] == IS31FL3737_PWM_HEAD);
    app_render(&app, 350u, pixels);
    assert(lit_pixels(pixels) == 0u);
    app_render(&app, 1099u, pixels);
    assert(lit_pixels(pixels) == 0u);
    app_render(&app, 1100u, pixels);
    assert_score_twelve(pixels);
    (void)short_start(&app, 1200u);
    assert(app.state == APP_PLAYING && app.game.score == 0u && app.high_score == 12u);
    app.game.food = 0u;
    app_update(&app, 1600u, 0u);
    assert(app.high_score == 12u);
    app_display_recovered(&app, 1700u, BUTTON_UP);
    app_render(&app, 1700u, pixels);
    assert_score_twelve(pixels);
    app_init(&app, 1800u, 0u, 0u);
    assert(app.high_score == 0u);
}

static void test_direction_chord_and_edge(void)
{
    app_t app;
    init_playing(&app, 0u);
    app_update(&app, 10u, BUTTON_UP | BUTTON_DOWN);
    app_update(&app, 30u, BUTTON_UP | BUTTON_DOWN);
    assert(!app.game.turn_pending);
    app_update(&app, 320u, BUTTON_UP | BUTTON_DOWN);
    assert(snake_head(&app.game) == 46u && app.game.direction == SNAKE_RIGHT);
    app_update(&app, 330u, 0u);
    app_update(&app, 350u, 0u);
    app_update(&app, 360u, BUTTON_UP);
    app_update(&app, 380u, BUTTON_UP);
    assert(app.game.turn_pending && app.game.pending_direction == SNAKE_UP);
    app_update(&app, 640u, BUTTON_UP);
    assert(snake_head(&app.game) == 36u && app.game.direction == SNAKE_UP);
    app_update(&app, 641u, BUTTON_UP);
    assert(!app.game.turn_pending);
}

static void test_no_catchup_and_recovery(void)
{
    app_t app;
    init_playing(&app, 0u);
    app_update(&app, 50000u, 0u);
    assert(snake_head(&app.game) == 46u && app.last_move == 50000u);
    app_update(&app, 50000u, 0u);
    app_update(&app, 50319u, 0u);
    assert(snake_head(&app.game) == 46u);
    app_update(&app, 50320u, 0u);
    assert(snake_head(&app.game) == 47u);
    app.high_score = 25u;
    app.start_tracking = true;
    app.start_long = true;
    app_display_recovered(&app, 60000u, BUTTON_START);
    assert(app.state == APP_READY && app.suppress_start);
    assert(!app.start_tracking && !app.start_long && app.high_score == 25u);
    app_update(&app, 70000u, BUTTON_START);
    assert(app.state == APP_READY && snake_head(&app.game) == 47u);
    app_update(&app, 70100u, 0u);
    app_update(&app, 70120u, 0u);
    assert(app.state == APP_READY && !app.suppress_start);
    (void)short_start(&app, 70200u);
    assert(app.state == APP_PLAYING && snake_head(&app.game) == 45u);
    assert(app.high_score == 25u);
}

static void test_won_transition(void)
{
    app_t app;
    uint8_t pixels[SNAKE_CAPACITY];
    uint8_t path[SNAKE_CAPACITY];
    unsigned row;
    unsigned col;
    unsigned i = 0u;
    for (row = 0u; row < SNAKE_HEIGHT; ++row) {
        for (col = 0u; col < SNAKE_WIDTH; ++col) {
            path[i++] = (uint8_t)(row * SNAKE_WIDTH +
                (row % 2u == 0u ? col : SNAKE_WIDTH - 1u - col));
        }
    }
    init_playing(&app, 0u);
    memcpy(app.game.cells, &path[1], SNAKE_CAPACITY - 1u);
    app.game.length = SNAKE_CAPACITY - 1u;
    app.game.score = SNAKE_MAX_SCORE - 1u;
    app.game.direction = SNAKE_LEFT;
    app_update(&app, 110u, 0u);
    assert(app.state == APP_WON && app.high_score == SNAKE_MAX_SCORE);
    app_render(&app, 110u, pixels);
    assert(lit_pixels(pixels) == SNAKE_CAPACITY);
    app_render(&app, 1010u, pixels);
    assert(lit_pixels(pixels) > 0u && lit_pixels(pixels) < SNAKE_CAPACITY);
    (void)short_start(&app, 1200u);
    assert(app.state == APP_PLAYING && app.game.length == SNAKE_INITIAL_LENGTH);
    assert(app.high_score == SNAKE_MAX_SCORE);
}

int main(void)
{
    test_per_key_debounce();
    test_uptime_wrap();
    test_boot_diagnostic_release();
    test_short_start_pause_resume();
    test_long_start_release_guard();
    test_wall_result_score_and_high_score();
    test_direction_chord_and_edge();
    test_no_catchup_and_recovery();
    test_won_transition();
    puts("PASS: 9 app/button test groups (debounce, wrap, control, render, recovery, win).");
    return 0;
}
