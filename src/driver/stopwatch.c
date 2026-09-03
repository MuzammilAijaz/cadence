#include <stdint.h>

#include <stm8s.h>
#include <stm8s_tim4.h>

#define SYSTEM_CLOCK_HZ       16000000UL // 16 MHz at max clock
#define TIM4_PRESCALER_VALUE  128UL
#define TICK_PERIOD_MS        1UL

/* Timer Clock Calculation:
 *
 * Timer clock = 16 MHz / 128 = 125 kHz
 * Timer tick  = 1 / 125 kHz  = 8 us
 *
 * if period   = 1 ms         = 1000 / 8 = 125 timer ticks
 * Auto-reload = 125 - 1      = 124
 */

#define TIMER_TICKS_PER_MS \
  (SYSTEM_CLOCK_HZ / TIM4_PRESCALER_VALUE / 1000UL)
/** The amount of ticks it takes to restart the timer */
#define TIM4_AUTORELOAD \
  ((TIMER_TICKS_PER_MS * TICK_PERIOD_MS) - 1UL)

/**
 * @brief Initialize TIM4 as a 1 ms system tick timer.
 *
 * Enables the TIM4 peripheral clock, configures TIM4 to overflow every
 * 1 ms, enables the TIM4 update interrupt, and starts the timer.
 *
 * With a 16 MHz system clock and a 128:1 prescaler, TIM4 runs at
 * 125 kHz. The resulting 8 us timer tick requires 125 timer ticks
 * for a 1 ms period.
 *
 * @note The TIM4 update interrupt must be handled by
 *       TIM4_UPD_OVF_IRQHandler().
 */
void stopwatch_init(void) {
  // explicit clock enable
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);

  TIM4_TimeBaseInit(TIM4_PRESCALER_128, (uint8_t)TIM4_AUTORELOAD);

  TIM4_ClearFlag(TIM4_FLAG_UPDATE); // safety
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE); // interrupt overflow
  TIM4_Cmd(ENABLE); // start timer
}
