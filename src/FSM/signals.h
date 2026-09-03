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
    STOPWATCH_ENABLE_SIG, // when the stopwatch is selected from menu
    STOPWATCH_DISABLE_SIG,
    START_WATCH_SIG,
    STOP_WATCH_SIG,
    TIMER_HIT_SIG,
    SM_TIMER_END,

    SM_LCD_START,
    DISPLAY_UPDATE_TICK_SIG,
    DISPLAY_TIMER_ON_STATUS,
    DISPLAY_TIMER_OFF_STATUS,
    SM_LCD_END,

    SM_AUDIO_START,
    AUDIO_TURN_ON_SIG,
    AUDIO_TURN_OFF_SIG,
    SM_AUDIO_END,

    SM_ADC_START,
    ADC_X_CONVERTED_SIG,
    ADC_Y_CONVERTED_SIG,
    SM_ADC_END,

} EventSignals;

#endif

