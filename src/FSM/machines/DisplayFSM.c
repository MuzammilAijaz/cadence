#include "DisplayFSM.h"
#include "lcd.h"
#include "signals.h"
#include "FSM.h"

#include "cadence_assert.h"

#include <string.h>

#include <stm8s_exti.h>
#include <stm8s.h>

#define TIMEBUF_LEN 9

typedef struct DisplayFSM {
  FSM super; // base class using "composition" in C

  unsigned long last_ms;
  char timebuf[TIMEBUF_LEN];
} DisplayFSM;

static void format_time(unsigned long ms, char *buf);

static State DisplayFSM_Running(DisplayFSM* const me, Event const * const e);

static DisplayFSM l_displayFSM;
FSM * g_displayFSM = NULL;

void DisplayFSM_ctor(void) {
  FSM_ctor(&l_displayFSM.super, (StateHandler)DisplayFSM_Running);

  l_displayFSM.last_ms = 0UL;
  memset(l_displayFSM.timebuf, 0, sizeof(l_displayFSM.timebuf));

  g_displayFSM = (FSM*) &l_displayFSM;

  lcd_init();
  lcd_reset_screen();

  // RESEARCH: perhaps let the CTOR do it?
  /* initialize the FSM */
  FSM_init(&l_displayFSM.super, (Event *)0);
}

/*---------------------------------------------------------------------------*/

static State DisplayFSM_Running(DisplayFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	status = HANDLED_STATUS;
      } break;

    case DISPLAY_ON_STATUS:
      {
	lcd_set_cursor(1, 0);
	lcd_print("[ GO ]");
	status = HANDLED_STATUS;
      } break;

    case DISPLAY_OFF_STATUS:
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

