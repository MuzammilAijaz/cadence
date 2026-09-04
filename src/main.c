#include <stm8s.h>
#include <stm8s_clk.h>
#include <stm8s_gpio.h>
#include <stm8s_exti.h>
#include <stm8s_tim4.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <cadence_assert.h>

#include "stm8s103f3_config.h"
#include "WatchFSM.h"
#include "DisplayFSM.h"
#include "AudioFSM.h"
#include "FSM.h"
#include "utils.h"
#include "led.h"
#include "stopwatch.h"
#include "button.h"
#include "analog_stick.h"
#include "audio.h"

// important so toolchain doesnt optimize code out.
extern void EXTI_PORTD_IRQHandler(void) __interrupt(6);
extern void TIM4_UPD_OVF_IRQHandler(void) __interrupt(23);
extern void ADC1_IRQHandler(void) __interrupt(22);
extern void TIM2_UPD_OVF_BRK_IRQHandler(void) __interrupt(13);

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
  CLK_HSIPrescalerConfig(SETTING_CLOCK_PRESCALER);

  /* @see driver initializations for clock initializations of other
   * peripherals.
   */

  //----- GPIO Initializations -----------------------------------

  GPIO_Init(AUDIO_PORT, AUDIO_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
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

  /* Init TIM2 and stop it immediately to avoid playing unecessary audio */
  audio_init();
  audio_stop();

  //----- FSM Construction ---------------------------------------

  WatchFSM_ctor();
  DisplayFSM_ctor();
  AudioFSM_ctor();

  //----- Run Main Event Loop ------------------------------------

  enableInterrupts();
  // Start event loop
  EventLoop();

  return 0; // should never end up here...
}

//==============================================================================

// See: https://community.st.com/s/question/0D50X00009XkhigSAB/what-is-the-purpose-of-define-usefullassert
#ifdef USE_FULL_ASSERT
static void blink_count(uint16_t count)
{
    for (uint16_t i = 0; i < count; i++) {
        GPIO_WriteLow(LED_PORT, LED_PIN);
        delay_ms(130);

        GPIO_WriteHigh(LED_PORT, LED_PIN);
        delay_ms(130);
    }

    delay_ms(700); // give 0.7 second to process per digit
}

/**
 * Notifies the developer about the line where the assertion occurred
 * using a decimal LED blink code.
 *
 * How to read the line number:
 * ----------------------------
 *  -> Wait for the LED to blink rapidly.
 *  -> A 1s pause indicates that the line number is about to be displayed.
 *  -> Count the blinks for each digit.
 *  -> A longer pause separates each digit.
 *
 *  The sequence repeats after a 2s pause.
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    (void) file;

    uint16_t hundreds;
    uint16_t tens;
    uint16_t ones;

    hundreds = line / 100;
    tens = (line / 10) % 10;
    ones = line % 10;

    while (TRUE)
    {
      /* Signal start of error */
      int iter = 15;
      while (iter--) {
	GPIO_WriteLow(LED_PORT, LED_PIN);
	delay_ms(20);
	GPIO_WriteHigh(LED_PORT, LED_PIN);
	delay_ms(20);
      }
      delay_ms(1000);

      /* Hundreds */
      if (hundreds) blink_count(hundreds);
      /* Tens */
      if (tens) blink_count(tens);
      /* Ones */
      blink_count(ones);

      /* Long pause before repeating */
      delay_ms(2000);
    }
}
#endif
