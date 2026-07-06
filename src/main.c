#include <stm8s.h>
#include <stm8s_clk.h>
#include <stm8s_gpio.h>
#include <stm8s_exti.h>

// Input Pins
#define BUTTON_PORT  GPIOD
#define BUTTON_PIN  GPIO_PIN_4

//----- LCD ----------------------------------------------------

// Control Pins
#define LCD_RS_PORT  GPIOD
#define LCD_RS_PIN   GPIO_PIN_3
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

//--------------------------------------------------------------

volatile int buttonPressed = 0;

// important so toolchain doesnt optimize code out.
extern void EXTI_PORTD_IRQHandler(void) __interrupt(6);

/* 16MHz CPU Delay helper (Roughly 1 millisecond per 'ms' unit) */
static void delay_ms(unsigned long ms) {
  unsigned long count = ms * 1600;
  while (count--) {
    __asm__("nop");
  }
}

//===== LCD controls ===========================================================

/* Toggle the Enable pin to latch data into the LCD */
static void lcd_pulse_enable(void) {
  // TODO: verify timing
  GPIO_WriteHigh(LCD_E_PORT, LCD_E_PIN);
  delay_ms(1); // Enable pulse width must be > 450ns, 1ms is perfectly safe
  GPIO_WriteLow(LCD_E_PORT, LCD_E_PIN);
  delay_ms(1); // Execution time safeguard
}

/* Send 4 bits of data to the LCD screen */
static void lcd_send_nibble(uint8_t nibble) {
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

//==============================================================================

int main(void)
{
  /* Run CPU at full 16 MHz clock speed */
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  /* Initialize the LCD screen */
  lcd_init();

  /* Button */
  GPIO_Init(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_IN_PU_IT);
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);

  /* Write character 'A' onto the screen */
  lcd_send_byte('A', 1);

  enableInterrupts();

  while (1)
  {
    if (buttonPressed) {
      buttonPressed = 0;
      lcd_send_byte('B', 1);
      delay_ms(200); // crude debounce
    }
  }
}

// See: https://community.st.com/s/question/0D50X00009XkhigSAB/what-is-the-purpose-of-define-usefullassert
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
	while (TRUE)
	{
	}
}
#endif
