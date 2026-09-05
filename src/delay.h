#ifndef DELAY__H
#define DELAY__H

/* 16MHz CPU Delay helper.
 *
 * @param[in] ms (Roughly 1 millisecond per 'ms' unit)
 */
static inline void delay_ms(unsigned long ms) {
    unsigned long count = ms * 1600;
    while (count--) {
        __asm__("nop");
    }
}

#endif

