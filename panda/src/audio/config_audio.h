// Filename: config_audio.h
// Created by:  cary (22Sep00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_AUDIO_H__
#define __CONFIG_AUDIO_H__

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(audio, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA int audio_sample_voices;
extern EXPCL_PANDA int audio_mix_freq;
extern EXPCL_PANDA string* audio_mode_flags;
extern EXPCL_PANDA int audio_driver_select;
extern EXPCL_PANDA string* audio_driver_params;
extern EXPCL_PANDA int audio_buffer_size;
extern EXPCL_PANDA string* audio_device;
extern EXPCL_PANDA int audio_auto_update_delay;
extern EXPCL_PANDA bool audio_is_active;
extern EXPCL_PANDA bool audio_sfx_active;
extern EXPCL_PANDA bool audio_music_active;
extern EXPCL_PANDA float audio_master_sfx_volume;
extern EXPCL_PANDA float audio_master_music_volume;
extern EXPCL_PANDA int audio_thread_priority;

extern EXPCL_PANDA void audio_load_loaders();

#ifndef NDEBUG //[
  // Non-release build:
  #define audio_debug(msg) \
  if (audio_cat.is_debug()) { \
    audio_cat->debug() << msg << endl; \
  } else {}

  #define audio_info(msg) \
    audio_cat->info() << msg << endl
#else //][
  // Release build:
  #define audio_debug(msg) ((void)0)
  #define audio_info(msg) ((void)0)
#endif //]

#define audio_error(msg) \
  audio_cat->error() << msg << endl

#endif /* __CONFIG_AUDIO_H__ */
