// Filename: config_audio.cxx
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "config_audio.h"
#include "audio_sample.h"
#include "audio_music.h"
#include <dconfig.h>

Configure(config_audio);
NotifyCategoryDef(audio, "");

int audio_sample_voices = config_audio.GetInt("audio-sample-voices", 8);
int audio_mix_freq = config_audio.GetInt("audio-mix-freq", 11025);
string* audio_mode_flags;
int audio_driver_select = config_audio.GetInt("audio-driver-select", 0);
string* audio_driver_params;

ConfigureFn(config_audio) {
  AudioSample::init_type();
  AudioMusic::init_type();

  Config::ConfigTable::Symbol mode;
  config_audio.GetAll("audio-mode-flag", mode);
  Config::ConfigTable::Symbol::iterator i;
  audio_mode_flags = new string;
  for (i=mode.begin(); i!=mode.end(); ++i) {
    if (!audio_mode_flags->empty())
      *audio_mode_flags += " ";
    *audio_mode_flags += (*i).Val();
  }
  Config::ConfigTable::Symbol parms;
  config_audio.GetAll("audio-driver-param", parms);
  audio_driver_params = new string;
  for (i=parms.begin(); i!=parms.end(); ++i) {
    if (!audio_driver_params->empty())
      *audio_driver_params += " ";
    *audio_driver_params += (*i).Val();
  }
}
