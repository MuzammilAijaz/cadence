/*****************************************************************************
 * State machine implementation inspired from lesson 39 of modern-embedded-
 * programming-course by Quantum Leaps.
 *
 * Copyright (C) 2020 Quantum Leaps, LLC.
 * Modified to remove RTOS dependencies.
 *
 * SPDX-License-Identifier: Apache-2.0
 *****************************************************************************/

#ifndef FSM__H
#define FSM__H

#include <stdint.h>

/*---------------------------------------------------------------------------*/
/* Event facilities... */

typedef uint16_t Signal; /* event signal */

/* Event base class */
typedef struct {
    Signal sig; /* event signal */
    /* event parameters added in subclasses of Event */
} Event;

/* Custom Event class */
typedef struct {
    Event super; // base clas

    uint16_t value; // ADC converted value // @see ADC1_GetConversionValue
} AdcEvent;

/*---------------------------------------------------------------------------*/
/* Finite State Machine facilities... */

typedef struct FSM FSM; /* forward declaration */

typedef enum { TRAN_STATUS, HANDLED_STATUS, IGNORED_STATUS, INIT_STATUS } State;

typedef State (*StateHandler)(FSM * const me, Event const * const e);

#define TRAN(target_) (((FSM *)me)->state = (StateHandler)(target_), TRAN_STATUS)

struct FSM {
    StateHandler state; /* the "state variable" / function */
};

void FSM_ctor(FSM * const me, StateHandler initial);
void FSM_init(FSM * const me, Event const * const e);
void FSM_dispatch(FSM * const me, Event const * const e);

void Event_post(Event const *e);
void EventLoop(void);

#endif

