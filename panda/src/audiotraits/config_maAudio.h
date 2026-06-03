/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_maAudio.h
 * @author Katie & J0y
 */

#ifndef CONFIG_MINIAUDIO_H
#define CONFIG_MINIAUDIO_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "audioManager.h"

ConfigureDecl(config_maAudio, EXPCL_MA_AUDIO, EXPTP_MA_AUDIO);
NotifyCategoryDecl(maAudio, EXPCL_MA_AUDIO, EXPTP_MA_AUDIO);

extern "C" EXPCL_MA_AUDIO void init_libMiniAudio();
extern "C" EXPCL_MA_AUDIO Create_AudioManager_proc *get_audio_manager_func_ma_audio();

extern ConfigVariableString ma_device;
extern ConfigVariableInt ma_buffer_delete_retries;
extern ConfigVariableDouble ma_buffer_delete_delay;

#endif // CONFIG_OPENALAUDIO_H
