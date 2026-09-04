#ifndef CADENCE_ASSERT_H
#define CADENCE_ASSERT_H

#include <stdint.h>
#include <stm8s_gpio.h>
#include <stm8s_conf.h>
#include "utils.h"
#include "led.h"

#define DEBUG_BUILD 1

#if defined(DEBUG_BUILD) && defined(USE_FULL_ASSERT)

#define CADENCE_ASSERT(expr)                     \
    do {                                   \
        if (!(expr)) {                     \
            assert_failed(__FILE__,      \
                    (uint16_t)__LINE__); \
        }                                  \
    } while (0)

#elif defined(DEBUG_BUILD)

#define CADENCE_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_FAST); \
            while (TRUE)  { \
                GPIO_WriteHigh(LED_PORT, LED_PIN); \
                delay_ms(200); \
                GPIO_WriteLow(LED_PORT, LED_PIN); \
                delay_ms(200); \
            } \
        } \
    } while (0)

#else

#define CADENCE_ASSERT(expr) ((void)0)

#endif

#endif
