/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_openalAudio.h
 * @author Ben Buchwald <bb2@alumni.cmu.edu>
 */

#ifndef CONFIG_OPENALAUDIO_H
#define CONFIG_OPENALAUDIO_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "audioManager.h"

ConfigureDecl(config_openalAudio, EXPCL_OPENAL_AUDIO, EXPTP_OPENAL_AUDIO);
NotifyCategoryDecl(openalAudio, EXPCL_OPENAL_AUDIO, EXPTP_OPENAL_AUDIO);

extern "C" EXPCL_OPENAL_AUDIO void init_libOpenALAudio();
extern "C" EXPCL_OPENAL_AUDIO Create_AudioManager_proc *get_audio_manager_func_openal_audio();

extern ConfigVariableString openal_device;
extern ConfigVariableInt openal_buffer_delete_retries;
extern ConfigVariableDouble openal_buffer_delete_delay;

#endif // CONFIG_OPENALAUDIO_H
