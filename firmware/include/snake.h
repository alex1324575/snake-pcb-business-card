#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include <stdint.h>

#define SNAKE_WIDTH 10u
#define SNAKE_HEIGHT 9u
#define SNAKE_CAPACITY (SNAKE_WIDTH * SNAKE_HEIGHT)
#define SNAKE_INITIAL_LENGTH 3u
#define SNAKE_MAX_SCORE (SNAKE_CAPACITY - SNAKE_INITIAL_LENGTH)
#define SNAKE_NO_CELL UINT8_MAX

typedef enum {
    SNAKE_UP = 0,
    SNAKE_RIGHT = 1,
    SNAKE_DOWN = 2,
    SNAKE_LEFT = 3
} snake_direction_t;

typedef enum {
    SNAKE_RUNNING = 0,
    SNAKE_GAME_OVER,
    SNAKE_WON
} snake_status_t;

typedef struct {
    /* Row-major cell numbers, head first; only [0, length) is occupied. */
    uint8_t cells[SNAKE_CAPACITY];
    uint8_t length;
    uint8_t score;
    uint8_t food;
    snake_direction_t direction;
    snake_direction_t pending_direction;
    snake_status_t status;
    bool turn_pending;
    uint32_t rng_state;
} snake_t;

/* Starts a fresh game, facing right, head at (5,4). Zero is a valid seed.
 * The app owns title screens, pause, input debounce, and elapsed-time handling.
 * All calls take a valid non-null game. Only this module should mutate it. */
void snake_init(snake_t *game, uint32_t seed);

/* Queues at most one perpendicular turn per movement step. Repeated, opposite,
 * invalid, or later turns are rejected. Returns true only when queued. */
bool snake_queue_turn(snake_t *game, snake_direction_t direction);

/* Advances exactly one cell. Terminal states are idempotent. */
snake_status_t snake_step(snake_t *game);

bool snake_occupies(const snake_t *game, uint8_t x, uint8_t y);
uint8_t snake_head(const snake_t *game);
uint8_t snake_food(const snake_t *game);

/* 320 ms initially, reduced by 10 ms per food, clamped to 110 ms. */
uint16_t snake_step_interval_ms(const snake_t *game);

#endif
