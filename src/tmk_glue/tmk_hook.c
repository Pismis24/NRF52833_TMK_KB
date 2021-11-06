#include <stdint.h>
#include <stdbool.h>
#include "hook.h"
#include "kb_evt.h"

/* Called periodically from the keyboard loop (very often!) */
/* Default behaviour: do nothing. */
void hook_keyboard_loop(void)
{
    trig_kb_event_param(KB_EVT_TMK_HOOK, TMK_HOOK_LOOP);
}

/* Called on matrix state change event (every keypress => often!) */
/* Default behaviour: do nothing. */
void hook_matrix_change(keyevent_t event)
{
    trig_kb_event_param(KB_EVT_TMK_HOOK, TMK_HOOK_MATRIX_CHG);
}