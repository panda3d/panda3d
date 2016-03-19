/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_milesAudio.h
 * @author skyler
 */

#ifndef CONFIG_MILESAUDIO_H
#define CONFIG_MILESAUDIO_H

#include "pandabase.h"

#ifdef HAVE_RAD_MSS //[
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "dconfig.h"
#include "audioManager.h"

ConfigureDecl(config_milesAudio, EXPCL_MILES_AUDIO, EXPTP_MILES_AUDIO);
NotifyCategoryDecl(milesAudio, EXPCL_MILES_AUDIO, EXPTP_MILES_AUDIO);

extern ConfigVariableBool miles_audio_force_midi_reset;
extern ConfigVariableInt miles_audio_expand_mp3_threshold;
extern ConfigVariableInt miles_audio_preload_threshold;
extern ConfigVariableBool miles_audio_panda_threads;

extern EXPCL_MILES_AUDIO void init_libMilesAudio();
extern "C" EXPCL_MILES_AUDIO Create_AudioManager_proc *get_audio_manager_func_miles_audio();
#endif //]

#endif // CONFIG_MILESAUDIO_H
