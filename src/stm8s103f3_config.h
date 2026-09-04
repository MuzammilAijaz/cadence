#ifndef STM8S103F3_CONFIG__H
#define STM8S103F3_CONFIG__H

#include <stm8s.h>
#include <stm8s_clk.h>
#include <stm8s_tim4.h>

//----- Hardware -----------------------------------------------

#define HSI_SOURCE_HZ               16000000UL

//----- Values -------------------------------------------------
// MODIFY these, appropriate settings are selected automatically.

// System clock
#define VALUE_SYSTEM_CLOCK_HZ       HSI_SOURCE_HZ

// Timer4 - stopwatch.c
#define VALUE_TIM4_TICK_PERIOD_MS 1UL

// Timer2 - audio.c
#define VALUE_TIM2_TICK_FREQUENCY_HZ 1000000UL // 1 million Hz, 1MHz, 1 microsecond

//----- Settings -----------------------------------------------
// DO NOT MODIFY!!
// Automatically selected based on values

#if (VALUE_SYSTEM_CLOCK_HZ == 16000000UL)
#define SETTING_CLOCK_PRESCALER CLK_PRESCALER_HSIDIV1
#else
#error "Invalid VALUE_SYSTEM_CLOCK_HZ selected!"
#endif

#if (VALUE_SYSTEM_CLOCK_HZ == 16000000UL) //--------------------

#if (VALUE_TIM4_TICK_PERIOD_MS == 1UL)
#define VALUE_TIM4_PRESCALER 128UL
#define SETTING_TIM4_PRESCALER      TIM4_PRESCALER_128
#else
#error "Invalid VALUE_TIM4_TICK_PERIOD_MS value selected!"
#endif

#if (VALUE_TIM2_TICK_FREQUENCY_HZ == 1000000UL)
#define SETTING_TIM2_PRESCALER  TIM2_PRESCALER_16
#else
#error "Unsupported timer tick frequency configuration!"
#endif

#endif //-------------------------------------------------------

//--------------------------------------------------------------

#endif
