#include "WatchFSM.h"
#include "signals.h"
#include "FSM.h"

#include <stdint.h>
#include <string.h>

#include <stm8s_exti.h>
#include <stm8s.h>

typedef struct WatchFSM {
  FSM super; // base class using "composition" in C
} WatchFSM;

static State WatchFSM_StopwatchStopped(WatchFSM* const me, Event const * const e);
static State WatchFSM_StopwatchRunning(WatchFSM* const me, Event const * const e);

static WatchFSM l_watchFSM;
FSM * g_watchFSM = NULL;

void WatchFSM_ctor(void) {
  FSM_ctor(&l_watchFSM.super, (StateHandler)WatchFSM_StopwatchStopped);

  g_watchFSM = (FSM*) &l_watchFSM;

  // RESEARCH: perhaps let the CTOR do it?
  /* initialize the FSM */
  FSM_init(&l_watchFSM.super, (Event *)0);
}

/*---------------------------------------------------------------------------*/

static State WatchFSM_StopwatchRunning(WatchFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	static Event const evt = { DISPLAY_ON_STATUS };
	Event_post(&evt);

	status = HANDLED_STATUS;
      } break;

    case TIMER_HIT_SIG:
      {
	static Event const evt = { DISPLAY_UPDATE_TICK_SIG };
	Event_post(&evt);
	status = HANDLED_STATUS;
      } break;

    case STOP_WATCH_SIG:
      {
	status = TRAN(WatchFSM_StopwatchStopped);
      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}

static State WatchFSM_StopwatchStopped(WatchFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	static Event const evt = { DISPLAY_OFF_STATUS };
	Event_post(&evt);

	status = HANDLED_STATUS;
      } break;

    case START_WATCH_SIG:
      {
	status = TRAN(WatchFSM_StopwatchRunning);
      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}

