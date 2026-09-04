#ifndef AUDIO__H
#define AUDIO__H

#include <stdint.h>
#include <stm8s_gpio.h>

#define AUDIO_PORT  GPIOD
#define AUDIO_PIN   GPIO_PIN_3

#define NOTE_REST_HZ  0
#define NOTE_C4_HZ    262
#define NOTE_CS4_HZ   277
#define NOTE_D4_HZ    294
#define NOTE_DS4_HZ   311
#define NOTE_E4_HZ    330
#define NOTE_F4_HZ    349
#define NOTE_FS4_HZ   370
#define NOTE_G4_HZ    392
#define NOTE_GS4_HZ   415
#define NOTE_A4_HZ    440
#define NOTE_AS4_HZ   466
#define NOTE_B4_HZ    494

#define NOTE_C5_HZ    523
#define NOTE_CS5_HZ   554
#define NOTE_D5_HZ    587
#define NOTE_DS5_HZ   622
#define NOTE_E5_HZ    659
#define NOTE_F5_HZ    698
#define NOTE_FS5_HZ   740
#define NOTE_G5_HZ    784
#define NOTE_GS5_HZ   831
#define NOTE_A5_HZ    880
#define NOTE_AS5_HZ   932
#define NOTE_B5_HZ    988

#define NOTE_C6_HZ    1047

typedef enum {
    NOTE_REST = 0,
    NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
    NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
    NOTE_C6,
    NOTE_MAX
} note_id_t;

void audio_init(void);
void audio_play_tone(uint16_t freq_hz);
void audio_stop(void);
void audio_play_melody(void);

#endif
