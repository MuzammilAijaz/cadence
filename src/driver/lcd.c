#include "lcd.h"
#include "utils.h"

#include <stm8s_gpio.h>

/* Toggle the Enable pin to latch data into the LCD */
void lcd_pulse_enable(void) {
  // TODO: verify timing
  GPIO_WriteHigh(LCD_E_PORT, LCD_E_PIN);
  delay_ms(1); // Enable pulse width must be > 450ns, 1ms is perfectly safe
  GPIO_WriteLow(LCD_E_PORT, LCD_E_PIN);
  delay_ms(1); // Execution time safeguard
}

/* Send 4 bits of data to the LCD screen */
void lcd_send_nibble(uint8_t nibble) {
  if (nibble & 0x01) GPIO_WriteHigh(LCD_D4_PORT, LCD_D4_PIN); else GPIO_WriteLow(LCD_D4_PORT, LCD_D4_PIN);
  if (nibble & 0x02) GPIO_WriteHigh(LCD_D5_PORT, LCD_D5_PIN); else GPIO_WriteLow(LCD_D5_PORT, LCD_D5_PIN);
  if (nibble & 0x04) GPIO_WriteHigh(LCD_D6_PORT, LCD_D6_PIN); else GPIO_WriteLow(LCD_D6_PORT, LCD_D6_PIN);
  if (nibble & 0x08) GPIO_WriteHigh(LCD_D7_PORT, LCD_D7_PIN); else GPIO_WriteLow(LCD_D7_PORT, LCD_D7_PIN);

  lcd_pulse_enable();
}

/* Send a full byte (Command or Data) split into two nibbles */
void lcd_send_byte(uint8_t byte, uint8_t is_data) {
  if (is_data) {
    GPIO_WriteHigh(LCD_RS_PORT, LCD_RS_PIN); // RS = 1 for Data
  } else {
    GPIO_WriteLow(LCD_RS_PORT, LCD_RS_PIN);  // RS = 0 for Command
  }

  lcd_send_nibble(byte >> 4);   // Higher 4 bits first
  lcd_send_nibble(byte & 0x0F);  // Lower 4 bits second
}

/* Initialization Flow */
void lcd_init(void) {
  // Power On
  GPIO_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(LCD_E_PORT,  LCD_E_PIN,  GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_MODE_OUT_PP_LOW_FAST);

  // Wait time > 15 ms after VDD > 4.5V
  delay_ms(20);

  GPIO_WriteLow(LCD_RS_PORT, LCD_RS_PIN);
  //			 // D7 D6 D5 D4
  lcd_send_nibble(0x03); // 0  0  1   1
  delay_ms(5); // > 4.1ms
  lcd_send_nibble(0x03);
  delay_ms(1); // > 100us
  lcd_send_nibble(0x03);

  lcd_send_nibble(0x02);
  lcd_send_byte(0x28, 0); // Function Set: 4-bit mode, 2 lines, 5x8 font
  lcd_send_byte(0x0C, 0); // Display Control: Display ON, Cursor OFF, Blink OFF
  lcd_send_byte(0x01, 0); // Clear Display
  lcd_send_byte(0x06, 0); // Entry Mode Set: Increment cursor automatically
}

/* Print a null-terminated string */
void lcd_print(const char *s) {
  while (*s) {
    lcd_send_byte(*s++, 1);
  }
}

/* Set cursor position (row 0-1, col 0-15) */
void lcd_set_cursor(uint8_t row, uint8_t col) {
  uint8_t addr = col + (row ? 0x40 : 0x00);
  lcd_send_byte(0x80 | addr, 0);
}

void lcd_reset_screen(void) {
  /* Draw static label */
  lcd_set_cursor(0, 0);
  lcd_print("Time");

  /* Show initial time */
  lcd_set_cursor(0, 8);
  lcd_print("00:00:00");

  /* Show STOP indicator */
  lcd_set_cursor(1, 0);
  lcd_print("[STOP]");
}

