// Filename: config_audio.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_AUDIO_H__
#define __CONFIG_AUDIO_H__

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(audio, EXPCL_PANDA, EXPTP_PANDA);

extern int audio_sample_voices;
extern int audio_mix_freq;
extern string* audio_mode_flags;
extern int audio_driver_select;
extern string* audio_driver_params;

#endif /* __CONFIG_AUDIO_H__ */
