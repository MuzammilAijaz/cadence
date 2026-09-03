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

static State WatchFSM_StopwatchDisabled(WatchFSM* const me, Event const * const e);
static State WatchFSM_StopwatchStopped(WatchFSM* const me, Event const * const e);
static State WatchFSM_StopwatchRunning(WatchFSM* const me, Event const * const e);

static WatchFSM l_watchFSM;
FSM * g_watchFSM = NULL;

void WatchFSM_ctor(void) {
  FSM_ctor(&l_watchFSM.super, (StateHandler)WatchFSM_StopwatchDisabled);

  g_watchFSM = (FSM*) &l_watchFSM;

  // RESEARCH: perhaps let the CTOR do it?
  /* initialize the FSM */
  FSM_init(&l_watchFSM.super, (Event *)0);
}

/*---------------------------------------------------------------------------*/

static State WatchFSM_StopwatchDisabled(WatchFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	status = HANDLED_STATUS;
      } break;

    case STOPWATCH_ENABLE_SIG:
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

static State WatchFSM_StopwatchRunning(WatchFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	static Event const evt = { DISPLAY_TIMER_ON_STATUS };
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

    // FIXME: requires letting the stm8s_it.c know 
    // stopwatch is NOT on now. Since we are trying to stop a watch
    // without pressing the button (we use analog stick)...
    // Result -> Requires 2 key presses instead of 1 when entering back
    //
    // REFACTOR: implement hsm to avoid this...
    case STOPWATCH_DISABLE_SIG:
      {
	status = TRAN(WatchFSM_StopwatchDisabled);
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
	static Event const evt = { DISPLAY_TIMER_OFF_STATUS };
	Event_post(&evt);

	status = HANDLED_STATUS;
      } break;

    case START_WATCH_SIG:
      {
	status = TRAN(WatchFSM_StopwatchRunning);
      } break;

    // REFACTOR: implement hsm to avoid this...
    case STOPWATCH_DISABLE_SIG:
      {
	status = TRAN(WatchFSM_StopwatchDisabled);
      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}

