#include "DisplayFSM.h"
#include "lcd.h"
#include "signals.h"
#include "FSM.h"

#include "cadence_assert.h"

#include <string.h>

#include <stm8s_exti.h>
#include <stm8s.h>

#define TIMEBUF_LEN 9

typedef enum {
  MENU_TIMER,
} MenuHighlightOption;

typedef struct DisplayFSM {
  FSM super; // base class using "composition" in C

  MenuHighlightOption current_highlight;

  // Timer State
  unsigned long last_ms;
  char timebuf[TIMEBUF_LEN];
} DisplayFSM;

static void displayMenu(DisplayFSM* const me);
static void format_time(unsigned long ms, char *buf);

static State DisplayFSM_MainMenu(DisplayFSM* const me, Event const * const e);
static State DisplayFSM_Timer(DisplayFSM* const me, Event const * const e);

static DisplayFSM l_displayFSM;
FSM * g_displayFSM = NULL;

void DisplayFSM_ctor(void) {
  FSM_ctor(&l_displayFSM.super, (StateHandler)DisplayFSM_MainMenu);

  l_displayFSM.last_ms = 0UL;
  l_displayFSM.current_highlight = MENU_TIMER;
  memset(l_displayFSM.timebuf, 0, sizeof(l_displayFSM.timebuf));

  g_displayFSM = (FSM*) &l_displayFSM;

  lcd_init();

  // RESEARCH: perhaps let the CTOR do it?
  /* initialize the FSM */
  FSM_init(&l_displayFSM.super, (Event *)0);
}

/*---------------------------------------------------------------------------*/

static State DisplayFSM_MainMenu(DisplayFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	// display the first highlight option
	displayMenu(me);

	status = HANDLED_STATUS;
      } break;

    //----- Selection of Item --------------------------------------

    case ADC_X_CONVERTED_SIG: // @see ADC1_IRQHandler
      {
	// downcast
	AdcEvent * xEvt = (AdcEvent*) e;

	// REFACTOR: remove magic values
	if (xEvt->value >= 800) { // moved right
	  status = TRAN(DisplayFSM_Timer);
	} else {
	  status = HANDLED_STATUS;
	}

      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}

static State DisplayFSM_Timer(DisplayFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	static Event const evt = { STOPWATCH_ENABLE_SIG };
	Event_post(&evt);

	lcd_set_default_timer_screen();

	status = HANDLED_STATUS;
      } break;

    // go back to main menu
    case ADC_X_CONVERTED_SIG:
      {
	// downcast
	AdcEvent * xEvt = (AdcEvent*) e;

	// REFACTOR: remove magic values
	if (xEvt->value <= 200) { // moved left
	  // disable stopwatch and move to Mainmenu
	  static Event const evt = { STOPWATCH_DISABLE_SIG };
	  Event_post(&evt);

	  status = TRAN(DisplayFSM_MainMenu);
	}
	else {
	  status = HANDLED_STATUS;
	}

      } break;

    case DISPLAY_TIMER_ON_STATUS:
      {
	lcd_set_cursor(1, 0);
	lcd_print("[ GO ]");
	status = HANDLED_STATUS;
      } break;

    case DISPLAY_TIMER_OFF_STATUS:
      {
	lcd_set_cursor(1, 0);
	lcd_print("[STOP]");
	status = HANDLED_STATUS;
      } break;

    case DISPLAY_UPDATE_TICK_SIG:
      {
	// 1000 is the time it takes for the timer in ms
	// NOTE: lcd takes 32ms to update???
	me->last_ms += 1000; // FIXME: magic number + risk of going out of sync
	format_time(me->last_ms, me->timebuf);
	lcd_set_cursor(0, 8);
	lcd_print(me->timebuf);

	status = HANDLED_STATUS;
      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}

//===== Helpers ================================================================

static void displayMenu(DisplayFSM* const me) {
  lcd_clear();

  // all menu selections have this
  lcd_set_cursor(0, 15);
  lcd_print(">");

  switch(me->current_highlight) {

    case MENU_TIMER:
      {
	// PERFORMANCE: repeated string literals...
	lcd_set_cursor(0, 5);
	lcd_print("Timer");

	lcd_set_cursor(1, 4);
	lcd_print("00:00:00");

      } break;

    default:
      {

      } break;
  }
}

/* Format time: HH:MM:SS */
static void format_time(unsigned long ms, char *buf) {
  CADENCE_ASSERT(buf[TIMEBUF_LEN - 1] == '\0');

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

