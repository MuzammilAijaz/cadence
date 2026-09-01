#include <stm8s.h>
#include <stm8s_clk.h>
#include <stm8s_gpio.h>
#include <stm8s_exti.h>
#include <stm8s_tim4.h>

//==============================================================================

//----- Button -------------------------------------------------

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

//==============================================================================

// Variables shared with interrupt handlers in stm8s_it.c
volatile int buttonPressed = 0;
volatile unsigned long tick_ms = 0;
volatile unsigned long sys_ms = 0; // global 
volatile int running = 0;

// important so toolchain doesnt optimize code out.
extern void EXTI_PORTD_IRQHandler(void) __interrupt(6);
extern void TIM4_UPD_OVF_IRQHandler(void) __interrupt(23);

static void lcd_pulse_enable(void);
static void lcd_send_nibble(uint8_t nibble);
static void lcd_send_byte(uint8_t byte, uint8_t is_data);
static void lcd_init(void);
static void lcd_print(const char *s);
static void lcd_set_cursor(uint8_t row, uint8_t col);
static void tim4_init(void);
static void format_time(unsigned long ms, char *buf);
static void delay_ms(unsigned long ms);

//==============================================================================

int main(void)
{
  char timebuf[9];
  unsigned long last_ms = 0;

  /* Run CPU at full 16 MHz clock speed */
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  // Peripheral inits
  lcd_init();

  GPIO_Init(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_IN_PU_IT);
  GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_WriteHigh(LED_PORT, LED_PIN);

  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);

  /* Start TIM4 for 1ms ticks */
  tim4_init();

  /* Draw static label */
  lcd_set_cursor(0, 0);
  lcd_print("Time");

  /* Show initial time */
  lcd_set_cursor(0, 8);
  lcd_print("00:00:00");

  /* Show STOP indicator */
  lcd_set_cursor(1, 0);
  lcd_print("[STOP]");

  enableInterrupts();

  while (1)
  {
    /* Handle button: toggle running */
    if (buttonPressed) {
      buttonPressed = 0;
      running = !running;

      /* Update status indicator on row 2 */
      lcd_set_cursor(1, 0);
      if (running) {
        lcd_print("[ GO ]");
      } else {
        lcd_print("[STOP]");
      }
    }

    /* Update display every second */
    disableInterrupts();
    unsigned long now = tick_ms;
    enableInterrupts();

    if ((now / 1000) != (last_ms / 1000)) {
      last_ms = now;
      format_time(now, timebuf);
      lcd_set_cursor(0, 8);
      lcd_print(timebuf);
    }
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
static void lcd_send_byte(uint8_t byte, uint8_t is_data) {
  if (is_data) {
    GPIO_WriteHigh(LCD_RS_PORT, LCD_RS_PIN); // RS = 1 for Data
  } else {
    GPIO_WriteLow(LCD_RS_PORT, LCD_RS_PIN);  // RS = 0 for Command
  }

  lcd_send_nibble(byte >> 4);   // Higher 4 bits first
  lcd_send_nibble(byte & 0x0F);  // Lower 4 bits second
}

/* Initialization Flow */
static void lcd_init(void) {
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
static void lcd_print(const char *s) {
  while (*s) {
    lcd_send_byte(*s++, 1);
  }
}

/* Set cursor position (row 0-1, col 0-15) */
static void lcd_set_cursor(uint8_t row, uint8_t col) {
  uint8_t addr = col + (row ? 0x40 : 0x00);
  lcd_send_byte(0x80 | addr, 0);
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

/* Format time: HH:MM:SS */
static void format_time(unsigned long ms, char *buf) {
  unsigned long total_secs = ms / 1000;
  uint8_t s = total_secs % 60;
  uint8_t m = (total_secs / 60) % 60;
  uint8_t h = (total_secs / 3600) % 100;

  buf[0] = '0' + (h / 10);
  buf[1] = '0' + (h % 10);
  buf[2] = ':';
  buf[3] = '0' + (m / 10);
  buf[4] = '0' + (m % 10);
  buf[5] = ':';
  buf[6] = '0' + (s / 10);
  buf[7] = '0' + (s % 10);
  buf[8] = '\0';
}

/* 16MHz CPU Delay helper.
 *
 * @param[in] ms (Roughly 1 millisecond per 'ms' unit)
 */
static void delay_ms(unsigned long ms) {
  unsigned long count = ms * 1600;
  while (count--) {
    __asm__("nop");
  }
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
