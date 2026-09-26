#include "snake.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void assert_valid(const snake_t *game)
{
    unsigned i;
    unsigned j;
    assert(game->length >= SNAKE_INITIAL_LENGTH);
    assert(game->length <= SNAKE_CAPACITY);
    assert(game->score == game->length - SNAKE_INITIAL_LENGTH);
    for (i = 0; i < game->length; ++i) {
        assert(game->cells[i] < SNAKE_CAPACITY);
        for (j = i + 1u; j < game->length; ++j) {
            assert(game->cells[i] != game->cells[j]);
        }
    }
    if (game->status == SNAKE_WON) {
        assert(game->length == SNAKE_CAPACITY);
        assert(game->food == SNAKE_NO_CELL);
    } else {
        assert(game->food < SNAKE_CAPACITY);
        assert(!snake_occupies(game, game->food % SNAKE_WIDTH,
                              game->food / SNAKE_WIDTH));
    }
}

/* Tests inject valid board states to exercise rare end-game geometries. */
static void fixture(snake_t *game, const uint8_t *cells, uint8_t length,
                    snake_direction_t direction, uint8_t food)
{
    snake_init(game, 123u);
    memcpy(game->cells, cells, length);
    game->length = length;
    game->score = (uint8_t)(length - SNAKE_INITIAL_LENGTH);
    game->direction = direction;
    game->pending_direction = direction;
    game->turn_pending = false;
    game->food = food;
    assert_valid(game);
}

static void test_init_and_food(void)
{
    snake_t a;
    snake_t b;
    uint32_t seed;
    bool food_seen[SNAKE_CAPACITY] = {false};
    unsigned free_cells_seen = 0;
    unsigned i;

    for (seed = 0; seed < 4096u; ++seed) {
        snake_init(&a, seed);
        snake_init(&b, seed);
        assert(memcmp(&a, &b, sizeof(a)) == 0);
        assert(a.status == SNAKE_RUNNING);
        assert(a.direction == SNAKE_RIGHT);
        assert(a.length == 3u && a.score == 0u);
        assert(snake_head(&a) == 45u);
        assert(a.cells[1] == 44u && a.cells[2] == 43u);
        assert(a.rng_state != 0u);
        assert(snake_food(&a) == a.food);
        assert_valid(&a);
        food_seen[a.food] = true;
    }
    for (i = 0; i < SNAKE_CAPACITY; ++i) {
        if (food_seen[i]) {
            ++free_cells_seen;
        }
    }
    /* Reachability check, not a statistical proof of PRNG quality. */
    assert(free_cells_seen == SNAKE_CAPACITY - SNAKE_INITIAL_LENGTH);
    assert(!snake_occupies(&a, SNAKE_WIDTH, 0u));
    assert(!snake_occupies(&a, 0u, SNAKE_HEIGHT));
    assert(!snake_occupies(&a, UINT8_MAX, UINT8_MAX));
}

static void test_turn_queue(void)
{
    snake_t game;
    snake_init(&game, 1u);
    game.food = 89u;
    assert(!snake_queue_turn(&game, SNAKE_LEFT));
    assert(!snake_queue_turn(&game, SNAKE_RIGHT));
    assert(!snake_queue_turn(&game, (snake_direction_t)-1));
    assert(!snake_queue_turn(&game, (snake_direction_t)4));
    assert(snake_queue_turn(&game, SNAKE_UP));
    /* Two key presses before a tick must not turn right directly into left. */
    assert(!snake_queue_turn(&game, SNAKE_LEFT));
    assert(!snake_queue_turn(&game, SNAKE_DOWN));
    assert(snake_step(&game) == SNAKE_RUNNING);
    assert(game.direction == SNAKE_UP && snake_head(&game) == 35u);
    assert(!game.turn_pending);
    assert(!snake_queue_turn(&game, SNAKE_DOWN));
    assert(snake_queue_turn(&game, SNAKE_LEFT));
    assert(snake_step(&game) == SNAKE_RUNNING);
    assert(game.direction == SNAKE_LEFT && snake_head(&game) == 34u);
    assert_valid(&game);
}

static void test_each_wall(void)
{
    static const uint8_t bodies[4][3] = {
        {5u, 15u, 25u}, {49u, 48u, 47u},
        {85u, 75u, 65u}, {40u, 41u, 42u}
    };
    unsigned direction;
    for (direction = 0; direction < 4u; ++direction) {
        snake_t game;
        snake_t dead;
        fixture(&game, bodies[direction], 3u,
                (snake_direction_t)direction, 0u);
        assert(snake_step(&game) == SNAKE_GAME_OVER);
        assert(memcmp(game.cells, bodies[direction], 3u) == 0);
        dead = game;
        assert(snake_step(&game) == SNAKE_GAME_OVER);
        assert(!snake_queue_turn(&game, SNAKE_UP));
        assert(memcmp(&game, &dead, sizeof(game)) == 0);
    }
}

static void test_body_collision(void)
{
    static const uint8_t body[] = {44u, 43u, 33u, 34u, 35u, 45u, 55u};
    snake_t game;
    fixture(&game, body, (uint8_t)sizeof(body), SNAKE_RIGHT, 89u);
    assert(snake_queue_turn(&game, SNAKE_UP));
    assert(snake_step(&game) == SNAKE_GAME_OVER);
    assert(memcmp(game.cells, body, sizeof(body)) == 0);
}

static void test_vacating_tail(void)
{
    static const uint8_t body[] = {44u, 43u, 33u, 34u};
    static const uint8_t expected[] = {34u, 44u, 43u, 33u};
    snake_t game;
    fixture(&game, body, (uint8_t)sizeof(body), SNAKE_RIGHT, 89u);
    assert(snake_queue_turn(&game, SNAKE_UP));
    assert(snake_step(&game) == SNAKE_RUNNING);
    assert(memcmp(game.cells, expected, sizeof(expected)) == 0);
    assert(game.length == sizeof(body));
    assert_valid(&game);
}

static void test_growth_and_speed(void)
{
    snake_t game;
    unsigned score;
    snake_init(&game, 5u);
    game.food = 46u;
    assert(snake_step_interval_ms(&game) == 320u);
    assert(snake_step(&game) == SNAKE_RUNNING);
    assert(snake_head(&game) == 46u);
    assert(game.length == 4u && game.score == 1u);
    assert(game.cells[3] == 43u);
    assert(snake_step_interval_ms(&game) == 310u);
    assert_valid(&game);
    for (score = 0; score <= SNAKE_MAX_SCORE; ++score) {
        game.score = (uint8_t)score;
        assert(snake_step_interval_ms(&game) ==
               (score < 21u ? 320u - 10u * score : 110u));
    }
}

static void test_last_food_wins(void)
{
    snake_t game;
    snake_t won;
    uint8_t path[SNAKE_CAPACITY];
    unsigned row;
    unsigned col;
    unsigned i = 0;
    /* Contiguous serpentine path fills the board; cell 0 is the last food. */
    for (row = 0; row < SNAKE_HEIGHT; ++row) {
        for (col = 0; col < SNAKE_WIDTH; ++col) {
            path[i++] = (uint8_t)(row * SNAKE_WIDTH +
                (row % 2u == 0u ? col : SNAKE_WIDTH - 1u - col));
        }
    }
    fixture(&game, &path[1], SNAKE_CAPACITY - 1u, SNAKE_LEFT, 0u);
    assert(snake_step(&game) == SNAKE_WON);
    assert(game.score == SNAKE_MAX_SCORE && game.length == SNAKE_CAPACITY);
    assert(snake_food(&game) == SNAKE_NO_CELL);
    assert(memcmp(game.cells, path, sizeof(path)) == 0);
    assert_valid(&game);
    won = game;
    assert(snake_step(&game) == SNAKE_WON);
    assert(!snake_queue_turn(&game, SNAKE_UP));
    assert(memcmp(&game, &won, sizeof(game)) == 0);
}

static void test_complete_game(void)
{
    uint8_t cycle[SNAKE_CAPACITY];
    snake_t game;
    unsigned i = 0;
    unsigned row;
    unsigned col;
    unsigned head_index = 2u;
    unsigned steps = 0u;
    /* A Hamiltonian cycle provides legal play for every randomly placed food:
     * right across row 0, then down/up columns 9..0 in rows 1..8. */
    for (col = 0; col < SNAKE_WIDTH; ++col) {
        cycle[i++] = (uint8_t)col;
    }
    for (col = SNAKE_WIDTH; col > 0u; --col) {
        const unsigned x = col - 1u;
        for (row = 1; row < SNAKE_HEIGHT; ++row) {
            const unsigned y = x % 2u != 0u ? row : SNAKE_HEIGHT - row;
            cycle[i++] = (uint8_t)(y * SNAKE_WIDTH + x);
        }
    }
    assert(i == SNAKE_CAPACITY);
    {
        const uint8_t body[] = {cycle[2], cycle[1], cycle[0]};
        fixture(&game, body, 3u, SNAKE_RIGHT, cycle[3]);
    }
    while (game.status == SNAKE_RUNNING && steps < 8100u) {
        const uint8_t head = snake_head(&game);
        const uint8_t next = cycle[(head_index + 1u) % SNAKE_CAPACITY];
        snake_direction_t direction;
        if (next / SNAKE_WIDTH < head / SNAKE_WIDTH) {
            direction = SNAKE_UP;
        } else if (next / SNAKE_WIDTH > head / SNAKE_WIDTH) {
            direction = SNAKE_DOWN;
        } else if (next < head) {
            direction = SNAKE_LEFT;
        } else {
            direction = SNAKE_RIGHT;
        }
        if (direction != game.direction) {
            assert(snake_queue_turn(&game, direction));
        }
        snake_step(&game);
        ++steps;
        head_index = (head_index + 1u) % SNAKE_CAPACITY;
        assert(game.status != SNAKE_GAME_OVER);
        assert(snake_head(&game) == next);
        assert_valid(&game);
    }
    assert(game.status == SNAKE_WON && steps < 8100u);
    printf("Complete-game simulation: %u steps, score %u.\n", steps, game.score);
}

int main(void)
{
    test_init_and_food();
    test_turn_queue();
    test_each_wall();
    test_body_collision();
    test_vacating_tail();
    test_growth_and_speed();
    test_last_food_wins();
    test_complete_game();
    puts("PASS: 8 Snake test groups (4096 seeded starts and full-board play).");
    return 0;
}
