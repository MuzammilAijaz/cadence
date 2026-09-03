#include "AudioFSM.h"
#include "lcd.h"
#include "signals.h"
#include "FSM.h"

#include "cadence_assert.h"

#include <string.h>

#include <stm8s_exti.h>
#include <stm8s.h>

typedef struct AudioFSM {
  FSM super; // base class using "composition" in C

} AudioFSM;

static State AudioFSM_Off(AudioFSM* const me, Event const * const e);
static State AudioFSM_On(AudioFSM* const me, Event const * const e);

static AudioFSM l_audioFSM;
FSM * g_audioFSM = NULL;

void AudioFSM_ctor(void) {
  FSM_ctor(&l_audioFSM.super, (StateHandler)AudioFSM_Off);

  g_audioFSM = (FSM*) &l_audioFSM;

  /* initialize the FSM */
  FSM_init(&l_audioFSM.super, (Event *)0);
}

/*---------------------------------------------------------------------------*/

static State AudioFSM_Off(AudioFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	status = HANDLED_STATUS;
      } break;

    case AUDIO_TURN_ON_SIG:
      {
	status = TRAN(AudioFSM_On);
      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}

static State AudioFSM_On(AudioFSM* const me, Event const * const e) {
  State status;

  switch(e->sig) {

    case ENTRY_SIG:
      {
	status = HANDLED_STATUS;
      } break;

    case AUDIO_TURN_OFF_SIG:
      {
	status = TRAN(AudioFSM_Off);
      } break;

    default:
      {
	status = IGNORED_STATUS;
      } break;
  }
  return status;
}
