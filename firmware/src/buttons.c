#include "buttons.h"

void buttons_init(buttons_t *buttons, uint32_t now, uint8_t raw)
{
    unsigned i;
    buttons->candidate = buttons->stable = (uint8_t)(raw & BUTTON_MASK);
    buttons->pressed = buttons->released = 0u;
    for (i = 0u; i < 5u; ++i) buttons->changed_at[i] = now;
}

void buttons_update(buttons_t *buttons, uint32_t now, uint8_t raw)
{
    unsigned i;
    raw &= BUTTON_MASK;
    buttons->pressed = buttons->released = 0u;
    for (i = 0u; i < 5u; ++i) {
        uint8_t mask = (uint8_t)(1u << i);
        if ((raw ^ buttons->candidate) & mask) {
            buttons->candidate ^= mask;
            buttons->changed_at[i] = now;
        }
        if (((buttons->candidate ^ buttons->stable) & mask) &&
            (uint32_t)(now - buttons->changed_at[i]) >= BUTTON_DEBOUNCE_MS) {
            buttons->stable ^= mask;
            if (buttons->stable & mask) buttons->pressed |= mask;
            else buttons->released |= mask;
        }
    }
}
