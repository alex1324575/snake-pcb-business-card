#include "snake.h"

#include <stddef.h>
#include <string.h>

_Static_assert(SNAKE_CAPACITY < UINT8_MAX, "Cell indices must fit in uint8_t");

static uint32_t random_next(snake_t *game)
{
    uint32_t value = game->rng_state;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->rng_state = value;
    return value;
}

static uint32_t random_below(snake_t *game, uint32_t bound)
{
    /* Xorshift visits all nonzero uint32_t values. Subtracting one gives a
     * domain of UINT32_MAX values; rejection avoids modulo bias for bound. */
    const uint32_t limit = UINT32_MAX - (UINT32_MAX % bound);
    uint32_t sample;
    do {
        sample = random_next(game) - 1u;
    } while (sample >= limit);
    return sample % bound;
}

static bool contains_cell(const snake_t *game, uint8_t cell, uint8_t count)
{
    uint8_t i;
    for (i = 0; i < count; ++i) {
        if (game->cells[i] == cell) {
            return true;
        }
    }
    return false;
}

static void place_food(snake_t *game)
{
    uint8_t cell;
    uint32_t free_index;
    if (game->length == SNAKE_CAPACITY) {
        game->food = SNAKE_NO_CELL;
        return;
    }
    free_index = random_below(game, SNAKE_CAPACITY - game->length);
    for (cell = 0; cell < SNAKE_CAPACITY; ++cell) {
        if (!contains_cell(game, cell, game->length)) {
            if (free_index == 0u) {
                game->food = cell;
                return;
            }
            --free_index;
        }
    }
}

void snake_init(snake_t *game, uint32_t seed)
{
    memset(game, 0, sizeof(*game));
    game->length = SNAKE_INITIAL_LENGTH;
    game->cells[0] = (SNAKE_HEIGHT / 2u) * SNAKE_WIDTH + SNAKE_WIDTH / 2u;
    game->cells[1] = game->cells[0] - 1u;
    game->cells[2] = game->cells[0] - 2u;
    game->direction = SNAKE_RIGHT;
    game->pending_direction = SNAKE_RIGHT;
    game->status = SNAKE_RUNNING;
    game->rng_state = seed != 0u ? seed : UINT32_C(0x6D2B79F5);
    place_food(game);
}

bool snake_queue_turn(snake_t *game, snake_direction_t direction)
{
    if (game->status != SNAKE_RUNNING || game->turn_pending ||
        (unsigned)direction > (unsigned)SNAKE_LEFT) {
        return false;
    }
    if (direction == game->direction ||
        (((unsigned)direction + 2u) % 4u) == (unsigned)game->direction) {
        return false;
    }
    game->pending_direction = direction;
    game->turn_pending = true;
    return true;
}

snake_status_t snake_step(snake_t *game)
{
    int x;
    int y;
    uint8_t next;
    uint8_t i;
    uint8_t occupied_count;
    bool grows;

    if (game->status != SNAKE_RUNNING) {
        return game->status;
    }
    if (game->turn_pending) {
        game->direction = game->pending_direction;
        game->turn_pending = false;
    }
    x = game->cells[0] % SNAKE_WIDTH;
    y = game->cells[0] / SNAKE_WIDTH;
    switch (game->direction) {
    case SNAKE_UP: --y; break;
    case SNAKE_RIGHT: ++x; break;
    case SNAKE_DOWN: ++y; break;
    case SNAKE_LEFT: --x; break;
    }
    if (x < 0 || x >= (int)SNAKE_WIDTH || y < 0 || y >= (int)SNAKE_HEIGHT) {
        game->status = SNAKE_GAME_OVER;
        return game->status;
    }
    next = (uint8_t)((unsigned)y * SNAKE_WIDTH + (unsigned)x);
    grows = next == game->food;
    /* The current tail vacates on a non-growing step, so entering it is legal. */
    occupied_count = grows ? game->length : (uint8_t)(game->length - 1u);
    if (contains_cell(game, next, occupied_count)) {
        game->status = SNAKE_GAME_OVER;
        return game->status;
    }
    if (grows) {
        ++game->length;
        ++game->score;
    }
    for (i = (uint8_t)(game->length - 1u); i > 0u; --i) {
        game->cells[i] = game->cells[i - 1u];
    }
    game->cells[0] = next;
    if (grows) {
        if (game->length == SNAKE_CAPACITY) {
            game->food = SNAKE_NO_CELL;
            game->status = SNAKE_WON;
        } else {
            place_food(game);
        }
    }
    return game->status;
}

bool snake_occupies(const snake_t *game, uint8_t x, uint8_t y)
{
    if (x >= SNAKE_WIDTH || y >= SNAKE_HEIGHT) {
        return false;
    }
    return contains_cell(game, (uint8_t)(y * SNAKE_WIDTH + x), game->length);
}

uint8_t snake_head(const snake_t *game)
{
    return game->cells[0];
}

uint8_t snake_food(const snake_t *game)
{
    return game->food;
}

uint16_t snake_step_interval_ms(const snake_t *game)
{
    const uint16_t reduction = (uint16_t)(game->score * 10u);
    return reduction >= 210u ? 110u : (uint16_t)(320u - reduction);
}
