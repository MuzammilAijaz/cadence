#ifndef AUDIO_FSM__H
#define AUDIO_FSM__H

#include "FSM.h"

void AudioFSM_ctor(void);

extern FSM* g_audioFSM; /** Opaque pointer */

#endif
