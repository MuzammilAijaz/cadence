/*****************************************************************************
 * State machine implementation inspired from lesson 39 of modern-embedded-
 * programming-course by Quantum Leaps.
 *
 * Copyright (C) 2020 Quantum Leaps, LLC.
 * Modified to remove RTOS dependencies.
 *
 * SPDX-License-Identifier: Apache-2.0
 *****************************************************************************/

#include "cadence_assert.h"
#include "FSM.h"
#include "signals.h"
#include "WatchFSM.h"
#include "DisplayFSM.h"

#include <stdlib.h>
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/* Event facilities... */

#define EVENT_QUEUE_SIZE 8
static const Event *l_eventQueue[EVENT_QUEUE_SIZE];
static uint8_t l_qHead = 0; /* represents the next free index */
static uint8_t l_qTail = 0; /* represents the index of the last event */

/*---------------------------------------------------------------------------*/
/* Finite State Machine facilities... */

Event const entryEvt = { .sig = ENTRY_SIG };
Event const exitEvt  = { .sig = EXIT_SIG };

void FSM_ctor(FSM * const me, StateHandler initial) {
  me->state = initial;
}

void FSM_init(FSM * const me, Event const * const e) {
  CADENCE_ASSERT(me->state != (StateHandler)0);
  (*me->state)(me, e);
  (*me->state)(me, &entryEvt);
}

void FSM_dispatch(FSM * const me, Event const * const e) {
  State status;
  StateHandler prev_state = me->state; /* save for later */

  CADENCE_ASSERT(me->state != (StateHandler)0);
  status = (*me->state)(me, e);

  if (status == TRAN_STATUS) { /* transition taken? */
    (*prev_state)(me, &exitEvt);
    (*me->state)(me, &entryEvt);
  }
}

/*---------------------------------------------------------------------------*/

/* Posts to single global event queue.
 *
 * Can be called from anywhere in the system (ISRs, other modules etc)
 */
void Event_post(Event const *e) {
  uint8_t nextHead = (l_qHead + 1) % EVENT_QUEUE_SIZE;
  if (nextHead != l_qTail) { // NOT full
    l_eventQueue[l_qHead] = e;
    l_qHead = nextHead;
  }
  else {
    CADENCE_ASSERT(0);
  }
}

/** Gets the oldest event from the event queue `l_eventQueue` */
bool EventQueue_get(Event const **e) {
  if (l_qTail == l_qHead) {
    return 0;
  }
  *e = l_eventQueue[l_qTail];
  l_qTail = (l_qTail + 1) % EVENT_QUEUE_SIZE;
  return 1;
}

/* Super loop for dispatching events to multiple FSMs. */
void EventLoop(void) {
  CADENCE_ASSERT(g_watchFSM != NULL);
  CADENCE_ASSERT(g_displayFSM != NULL);

  /* event loop ("message pump") */
  while (1) {
    Event const *e; /* pointer to event object ("message") */

    if (EventQueue_get(&e)) {
      /* dispatch event to the FSM object */

      if (e->sig >= SM_TIMER_START && e->sig <= SM_TIMER_END) {
	FSM_dispatch(g_watchFSM, e);
      }

      else if (e->sig >= SM_LCD_START && e->sig <= SM_LCD_END) {
	FSM_dispatch(g_displayFSM, e);
      }
    }
  }
}
