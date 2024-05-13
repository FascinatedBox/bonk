#include "bonk_internal.h"

extern int b_set_window(bonk_state_t *);

int b_prop_adjust(bonk_state_t *b)
{
    return b_set_window(b);
}
