#include "common.h"

static _Context *do_event(_Event e, _Context *c) {
    switch (e.event) {
        /* PA 3.1 */
        case _EVENT_YIELD:
            Log("Event yield");
            break;
        /* PA 3.2 */
        case _EVENT_SYSCALL:
            return do_syscall(c);
            break;
        default:
            panic("Unhandled event ID = %d", e.event);
    }

    return NULL;
}

void init_irq(void) {
    Log("Initializing interrupt/exception handler...");
    _cte_init(do_event);
}
