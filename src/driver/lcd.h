#ifndef LCD__H
#define LCD__H

#include <stdint.h>

// Control Pins
#define LCD_RS_PORT  GPIOD
#define LCD_RS_PIN   GPIO_PIN_2
#define LCD_E_PORT   GPIOC
#define LCD_E_PIN    GPIO_PIN_7

// Data Pins (4-bit mode)
#define LCD_D4_PORT  GPIOA
#define LCD_D4_PIN   GPIO_PIN_3
#define LCD_D5_PORT  GPIOC
#define LCD_D5_PIN   GPIO_PIN_4
#define LCD_D6_PORT  GPIOC
#define LCD_D6_PIN   GPIO_PIN_3
#define LCD_D7_PORT  GPIOC
#define LCD_D7_PIN   GPIO_PIN_6

void lcd_pulse_enable(void);
void lcd_send_nibble(uint8_t nibble);
void lcd_send_byte(uint8_t byte, uint8_t is_data);
void lcd_init(void);
void lcd_print(const char *s);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_set_default_timer_screen(void);
void lcd_clear(void);

#endif
