// Filename: config_audio.h
// Created by:  cary (22Sep00)
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

extern EXPCL_PANDA void audio_load_loaders(void);

#endif /* __CONFIG_AUDIO_H__ */
