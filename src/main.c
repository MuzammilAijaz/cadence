#include <stm8s.h>
#include <stm8s_clk.h>
#include <stm8s_gpio.h>
#include <stm8s_exti.h>
#include <stm8s_tim4.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <cadence_assert.h>

#include "WatchFSM.h"
#include "DisplayFSM.h"
#include "FSM.h"
#include "utils.h"
#include "led.h"
#include "button.h"

// important so toolchain doesnt optimize code out.
extern void EXTI_PORTD_IRQHandler(void) __interrupt(6);
extern void TIM4_UPD_OVF_IRQHandler(void) __interrupt(23);

static void tim4_init(void);

int main(void)
{
  /* Run CPU at full 16 MHz clock speed */
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  GPIO_Init(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_IN_PU_IT);
  GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_WriteHigh(LED_PORT, LED_PIN);

  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);

  /* Start TIM4 for 1ms ticks */
  tim4_init();

  enableInterrupts();

  WatchFSM_ctor();
  DisplayFSM_ctor();

  // Start event loop
  EventLoop();

  return 0; // should never end up here...
}

//===== Helpers ================================================================

/* 
 * Setup TIM4 to overflow every 1ms at 16MHz.
 *
 * Prescaler=128 -> 125kHz tick,
 * Period=124 -> 1ms overflow
 */
static void tim4_init(void) {

  /* - 16 MHz cpu clock -> 16,000,000 / 128 (prescalar) = 125,000 Hz
   * - 1 / 125,000 = one timer tick is 8 micro seconds
   * - 124 is max val it counts to; 124(+1) * 8 = 1000us timer resets */
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);

  TIM4_ClearFlag(TIM4_FLAG_UPDATE); // safety
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE); // interrupt generation on overflow
  TIM4_Cmd(ENABLE); // start timer
}

//==============================================================================

// See: https://community.st.com/s/question/0D50X00009XkhigSAB/what-is-the-purpose-of-define-usefullassert
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
	while (TRUE)
	{
	  GPIO_WriteHigh(LED_PORT, LED_PIN);
	  delay_ms(100);

	  GPIO_WriteLow(LED_PORT, LED_PIN);
	  delay_ms(100);
	}
}
#endif
