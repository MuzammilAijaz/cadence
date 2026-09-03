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
#include "stopwatch.h"
#include "button.h"
#include "analog_stick.h"

// important so toolchain doesnt optimize code out.
extern void EXTI_PORTD_IRQHandler(void) __interrupt(6);
extern void TIM4_UPD_OVF_IRQHandler(void) __interrupt(23);

int main(void)
{
  //----- Clock --------------------------------------------------

  /*
   * Startup clock: After reset, the microcontroller restarts by
   * default with an internal 2 MHz clock (HSI/8).
   *
   * STM8S Peripheral Clock Gating:
   * ------------------------------
   *
   * CLK_PCKENR1:
   *   PCKEN23 -> ADC
   *   PCKEN22 -> AWU
   *   PCKEN17 -> TIM1
   *   PCKEN15 -> TIM2
   *   PCKEN14 -> TIM4
   *   PCKEN13 -> UART1
   *   PCKEN11 -> SPI
   *   PCKEN10 -> I2C
   *
   * NOTE: Unlike stm32, GPIO ports are NOT controlled by this
   * peripheral clock-gating mechanism.
   * 
   * NOTE: CLK_PCKENR1 is 0xFF by default (ungated)
   */

  /* Run CPU at FULL 16 MHz clock speed */
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  /* @see driver initializations for clock initializations of other
   * peripherals.
   */

  //----- GPIO Initializations -----------------------------------

  GPIO_Init(ANALOG_STICK_AXIS_X_PORT, ANALOG_STICK_AXIS_X_PIN, GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(ANALOG_STICK_AXIS_Y_PORT, ANALOG_STICK_AXIS_Y_PIN, GPIO_MODE_IN_FL_NO_IT);
  GPIO_Init(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_IN_PU_IT);
  GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_WriteHigh(LED_PORT, LED_PIN);

  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);

  //----- Driver Initializations ---------------------------------

  /* Start TIM4 ticks */
  stopwatch_init();
  /* Start ADC1 with interrupts */
  analog_stick_init();

  //----- FSM Construction ---------------------------------------

  WatchFSM_ctor();
  DisplayFSM_ctor();

  //----- Run Main Event Loop ------------------------------------

  enableInterrupts();
  // Start event loop
  EventLoop();

  return 0; // should never end up here...
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
