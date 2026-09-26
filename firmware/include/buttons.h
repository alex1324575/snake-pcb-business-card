#ifndef BUTTONS_H
#define BUTTONS_H
#include <stdint.h>
#define BUTTON_UP    (1u << 0)
#define BUTTON_DOWN  (1u << 1)
#define BUTTON_LEFT  (1u << 2)
#define BUTTON_RIGHT (1u << 3)
#define BUTTON_START (1u << 4)
#define BUTTON_MASK  31u
#define BUTTON_DEBOUNCE_MS 20u
typedef struct {
    uint32_t changed_at[5];
    uint8_t candidate, stable, pressed, released;
} buttons_t;
void buttons_init(buttons_t *buttons, uint32_t now, uint8_t raw);
void buttons_update(buttons_t *buttons, uint32_t now, uint8_t raw);
#endif
