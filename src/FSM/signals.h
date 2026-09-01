#ifndef SIGNALS__H
#define SIGNALS__H

enum ReservedSignals {
    INIT_SIG, /* dispatched to AO before entering event-loop */
    ENTRY_SIG, /* for triggering the entry action in a state */
    EXIT_SIG,  /* for triggering the exit action from a state */
    USER_SIG  /* first signal available to the users */
};

typedef enum {
    SM_TIMER_START = USER_SIG,
    START_WATCH_SIG,
    STOP_WATCH_SIG,
    TIMER_HIT_SIG,
    SM_TIMER_END,

    SM_LCD_START,
    DISPLAY_UPDATE_TICK_SIG,
    DISPLAY_ON_STATUS,
    DISPLAY_OFF_STATUS,
    SM_LCD_END,

} EventSignals;

#endif

