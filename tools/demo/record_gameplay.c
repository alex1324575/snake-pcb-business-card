/* Host-side demo recorder. Link the unmodified production app/snake/buttons.
 * The controller can only press/release the five physical buttons. It reads
 * game state to plan a route, but never writes state, food, score, or cells.
 * stdout: one compact JSON object per 80 ms rendered frame; stderr: summary.
 */
#include "app.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_MS 80u
#define START_AT_MS 1200u
#define PRESS_MS 40u
#define TARGET_SCORE 4u
#define RESULT_HOLD_MS 2200u
#define MAX_DURATION_MS 24000u

typedef struct {
    uint32_t seen_move, press_until;
    uint8_t press_button;
    unsigned turns, pickups;
    bool finishing;
} controller_t;

typedef struct {
    uint32_t seed, end_ms, game_over_ms;
    unsigned score, length, turns, pickups;
    bool wall_collision, valid;
} result_t;

static const int dx[4] = {0, 1, 0, -1};
static const int dy[4] = {-1, 0, 1, 0};
static const uint8_t button_for_direction[4] = {
    BUTTON_UP, BUTTON_RIGHT, BUTTON_DOWN, BUTTON_LEFT
};
static const char *const state_names[] = {
    "splash", "ready", "playing", "paused", "game_over", "won", "diagnostic"
};

/* The current tail may vacate. Blocking the rest of the present body is a
 * deliberately conservative route planner; it is not the game's collision
 * implementation. The actual app still accepts/rejects every move itself. */
static int direction_to_food(const snake_t *game)
{
    int queue[SNAKE_CAPACITY], first[SNAKE_CAPACITY];
    bool blocked[SNAKE_CAPACITY] = {false};
    bool seen[SNAKE_CAPACITY] = {false};
    int begin = 0, end = 0;
    unsigned i;
    for (i = 1u; i + 1u < game->length; ++i)
        blocked[game->cells[i]] = true;
    queue[end++] = (int)snake_head(game);
    seen[snake_head(game)] = true;
    first[snake_head(game)] = -1;
    while (begin < end) {
        int cell = queue[begin++];
        int d;
        /* Prefer continuing straight when two routes have equal length. */
        for (d = 0; d < 4; ++d) {
            int direction = ((int)game->direction + d) % 4;
            int x, y, next;
            if (cell == (int)snake_head(game) &&
                direction == (((int)game->direction + 2) % 4)) continue;
            x = cell % (int)SNAKE_WIDTH + dx[direction];
            y = cell / (int)SNAKE_WIDTH + dy[direction];
            if (x < 0 || y < 0 || x >= (int)SNAKE_WIDTH ||
                y >= (int)SNAKE_HEIGHT) continue;
            next = y * (int)SNAKE_WIDTH + x;
            if (blocked[next] || seen[next]) continue;
            first[next] = cell == (int)snake_head(game) ? direction : first[cell];
            if (next == (int)game->food) return first[next];
            seen[next] = true;
            queue[end++] = next;
        }
    }
    return (int)game->direction;
}

static bool next_move_is_wall(const snake_t *game)
{
    int direction = game->turn_pending ? (int)game->pending_direction : (int)game->direction;
    int x = (int)(snake_head(game) % SNAKE_WIDTH) + dx[direction];
    int y = (int)(snake_head(game) / SNAKE_WIDTH) + dy[direction];
    return x < 0 || y < 0 || x >= (int)SNAKE_WIDTH || y >= (int)SNAKE_HEIGHT;
}

static uint8_t raw_input(controller_t *controller, const app_t *app, uint32_t now)
{
    if (now >= START_AT_MS && now < START_AT_MS + PRESS_MS) return BUTTON_START;
    if (app->state != APP_PLAYING) return 0u;
    if (controller->seen_move != app->last_move) {
        int direction;
        controller->seen_move = app->last_move;
        if (app->game.score >= TARGET_SCORE) controller->finishing = true;
        direction = controller->finishing ? (int)app->game.direction : direction_to_food(&app->game);
        if (direction != (int)app->game.direction) {
            controller->press_button = button_for_direction[direction];
            controller->press_until = now + PRESS_MS;
            ++controller->turns;
        }
    }
    return now < controller->press_until ? controller->press_button : 0u;
}

static void print_frame(const app_t *app, uint32_t now, uint8_t raw)
{
    uint8_t pixels[SNAKE_CAPACITY];
    unsigned i;
    app_render(app, now, pixels);
    printf("{\"time_ms\":%lu,\"state\":\"%s\",\"score\":%u,\"length\":%u,"
           "\"head\":%u,\"food\":%u,\"direction\":%u,\"buttons\":%u,"
           "\"stable_buttons\":%u,\"last_move_ms\":%lu,\"state_since_ms\":%lu,\"cells\":[",
           (unsigned long)now, state_names[(unsigned)app->state],
           (unsigned)app->game.score, (unsigned)app->game.length,
           (unsigned)snake_head(&app->game), (unsigned)app->game.food,
           (unsigned)app->game.direction, (unsigned)raw,
           (unsigned)app->buttons.stable, (unsigned long)app->last_move,
           (unsigned long)app->state_since);
    for (i = 0u; i < app->game.length; ++i) printf("%s%u", i ? "," : "", (unsigned)app->game.cells[i]);
    printf("],\"pixels\":[");
    for (i = 0u; i < SNAKE_CAPACITY; ++i) printf("%s%u", i ? "," : "", (unsigned)pixels[i]);
    printf("]}\n");
}

static result_t run(uint32_t seed, bool record)
{
    app_t app;
    controller_t controller;
    result_t result;
    uint32_t now;
    unsigned previous_score = 0u;
    memset(&controller, 0, sizeof(controller));
    memset(&result, 0, sizeof(result));
    controller.seen_move = UINT32_MAX;
    result.seed = seed;
    app_init(&app, 0u, seed, 0u);
    for (now = 0u; now <= MAX_DURATION_MS; ++now) {
        uint8_t raw = raw_input(&controller, &app, now);
        bool would_hit_wall = app.state == APP_PLAYING && next_move_is_wall(&app.game);
        app_state_t old_state = app.state;
        app_update(&app, now, raw);
        if (app.game.score > previous_score) {
            ++controller.pickups;
            previous_score = app.game.score;
        }
        if (old_state == APP_PLAYING && app.state == APP_GAME_OVER) {
            result.game_over_ms = now;
            result.wall_collision = would_hit_wall;
        }
        if (record && now % FRAME_MS == 0u) print_frame(&app, now, raw);
        if (app.state == APP_GAME_OVER && now % FRAME_MS == 0u &&
            now - app.state_since >= RESULT_HOLD_MS) {
            result.end_ms = now;
            result.score = app.game.score;
            result.length = app.game.length;
            result.turns = controller.turns;
            result.pickups = controller.pickups;
            result.valid = result.wall_collision && result.score == TARGET_SCORE &&
                           result.length == SNAKE_INITIAL_LENGTH + TARGET_SCORE &&
                           controller.pickups == TARGET_SCORE && controller.turns >= 6u;
            return result;
        }
    }
    result.end_ms = MAX_DURATION_MS;
    return result;
}

int main(int argc, char **argv)
{
    uint32_t seed = 0u;
    result_t result;
    if (argc > 1) seed = (uint32_t)strtoul(argv[1], NULL, 0);
    if (seed == 0u) {
        uint32_t candidate;
        unsigned best_distance = UINT_MAX;
        for (candidate = 1u; candidate <= 1000u; ++candidate) {
            result_t trial = run(candidate, false);
            if (trial.valid && trial.end_ms >= 12000u && trial.end_ms <= 18000u) {
                unsigned distance = trial.end_ms >= 15000u ? trial.end_ms - 15000u : 15000u - trial.end_ms;
                if (distance < best_distance) {
                    best_distance = distance;
                    seed = candidate;
                }
            }
        }
        if (seed == 0u) {
            fprintf(stderr, "No eligible deterministic demonstration seed found.\n");
            return 2;
        }
    }
    result = run(seed, true);
    fprintf(stderr, "{\"seed\":%lu,\"end_ms\":%lu,\"game_over_ms\":%lu,\"score\":%u,"
                    "\"length\":%u,\"turns\":%u,\"pickups\":%u,\"wall_collision\":%s,\"valid\":%s}\n",
            (unsigned long)result.seed, (unsigned long)result.end_ms,
            (unsigned long)result.game_over_ms, result.score, result.length,
            result.turns, result.pickups, result.wall_collision ? "true" : "false",
            result.valid ? "true" : "false");
    return result.valid ? 0 : 3;
}
