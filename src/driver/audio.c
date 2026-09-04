#include "audio.h"
#include "utils.h"
#include <stm8s_tim2.h>
#include <stm8s_clk.h>

#include "stm8s103f3_config.h"

/*
 * @brief Calculates the no. of ticks to toggle gpio / reach half a cycle taking
 * into consideration that GPIOs will need to toggle twice per `freq_hz`.
 *
 * Standard timer ARR formula:
 *
 *     ARR = (TIMER_CLOCK_HZ / EVENT_FREQUENCY_HZ) - 1
 *
 * The TIM2 update event toggles the GPIO once.
 * A complete square-wave audio cycle requires 2 GPIO toggles:
 *
 *     EVENT_FREQUENCY_HZ = AUDIO_FREQUENCY_HZ * 2
 *
 * Therefore:
 *
 *     ARR = (TIMER_CLOCK_HZ / (AUDIO_FREQUENCY_HZ * 2)) - 1
 *
 * Example:
 *
 *     TIMER_CLOCK_HZ = 1,000,000 Hz
 *     AUDIO_FREQUENCY_HZ = 1,000 Hz
 *
 *     ARR = 1,000,000 / (1,000 * 2) - 1
 *         = 499
 *
 * TIM2 counts 0..499 = 500 ticks between GPIO toggles.
 *
 * With a 1 MHz timer clock, each tick is 1 us, so the GPIO
 * toggles every 500 us, producing a 1 kHz square wave.
 */
#define CALCULATE_AUDIO_AUTORELOAD(freq_hz) \
  (uint16_t)((VALUE_TIM2_TICK_FREQUENCY_HZ / \
        ((freq_hz) * 2UL)) - 1)

// Compile-time generated flash lookup table
static const uint16_t NOTE_ARR_LUT[NOTE_MAX] = {
  [NOTE_REST] = 0,
  [NOTE_C4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_C4_HZ),
  [NOTE_CS4]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_CS4_HZ),
  [NOTE_D4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_D4_HZ),
  [NOTE_DS4]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_DS4_HZ),
  [NOTE_E4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_E4_HZ),
  [NOTE_F4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_F4_HZ),
  [NOTE_FS4]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_FS4_HZ),
  [NOTE_G4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_G4_HZ),
  [NOTE_GS4]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_GS4_HZ),
  [NOTE_A4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_A4_HZ),
  [NOTE_AS4]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_AS4_HZ),
  [NOTE_B4]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_B4_HZ),

  [NOTE_C5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_C5_HZ),
  [NOTE_CS5]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_CS5_HZ),
  [NOTE_D5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_D5_HZ),
  [NOTE_DS5]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_DS5_HZ),
  [NOTE_E5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_E5_HZ),
  [NOTE_F5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_F5_HZ),
  [NOTE_FS5]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_FS5_HZ),
  [NOTE_G5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_G5_HZ),
  [NOTE_GS5]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_GS5_HZ),
  [NOTE_A5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_A5_HZ),
  [NOTE_AS5]  = CALCULATE_AUDIO_AUTORELOAD(NOTE_AS5_HZ),
  [NOTE_B5]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_B5_HZ),

  [NOTE_C6]   = CALCULATE_AUDIO_AUTORELOAD(NOTE_C6_HZ),
};

void audio_init(void) {
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);

  // if TIM2 prescaler is 16 and clock is 16MHz:
  //    then 16MHz / 16 = 1MHz tick (1us per tick)
  TIM2_TimeBaseInit(SETTING_TIM2_PRESCALER, 999); // 999 is just some random default ARR val
  TIM2_ClearFlag(TIM2_FLAG_UPDATE);
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
  TIM2_Cmd(ENABLE);
}

void audio_play_freq(uint16_t freq_hz) {
  if (freq_hz == 0) {
    audio_stop();
    return;
  }
  uint16_t period = CALCULATE_AUDIO_AUTORELOAD(freq_hz);
  TIM2_SetAutoreload(period);
  TIM2_Cmd(ENABLE);
}

void audio_play_note(note_id_t note) {
  if (note == NOTE_REST || note >= NOTE_MAX) {
    audio_stop();
    return;
  }
  // Single array lookup in Flash memory
  TIM2_SetAutoreload(NOTE_ARR_LUT[note]);
  TIM2_Cmd(ENABLE);
}

void audio_stop(void) {
  TIM2_Cmd(DISABLE);
  GPIO_WriteLow(AUDIO_PORT, AUDIO_PIN);
}

void audio_play_melody(void) {
  audio_play_note(NOTE_C5);
  delay_ms(50);

  audio_play_note(NOTE_E5);
  delay_ms(50);

  audio_play_note(NOTE_G5);
  delay_ms(100);

  audio_stop();
}
