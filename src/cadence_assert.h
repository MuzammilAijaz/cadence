#ifndef CADENCE_ASSERT_H
#define CADENCE_ASSERT_H

#include <stdint.h>
#include <stm8s_conf.h>

#define DEBUG_BUILD

#ifdef DEBUG_BUILD

#define CADENCE_ASSERT(expr)                     \
    do {                                   \
        if (!(expr)) {                     \
            assert_failed(__FILE__,      \
                    (uint16_t)__LINE__); \
        }                                  \
    } while (0)

#else

#define CADENCE_ASSERT(expr) ((void)0)

#endif

#endif
