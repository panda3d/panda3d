// Filename: config_audio.cxx
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

#include "config_audio.h"
#include "audio_sound.h"
#include "audio_gui_functor.h"
#include <dconfig.h>
#include <filename.h>
#include <load_dso.h>

Configure(config_audio);
NotifyCategoryDef(audio, "");

int audio_sample_voices = config_audio.GetInt("audio-sample-voices", 8);
int audio_mix_freq = config_audio.GetInt("audio-mix-freq", 11025);
string* audio_mode_flags;
int audio_driver_select = config_audio.GetInt("audio-driver-select", 0);
string* audio_driver_params;
int audio_buffer_size = config_audio.GetInt("audio-buffer-size", 4096);
string* audio_device;
int audio_auto_update_delay = config_audio.GetInt("audio-auto-update-delay",
                                                  1000000);
bool audio_is_active = config_audio.GetBool("audio-is-active", true);
bool audio_sfx_active = config_audio.GetBool("audio-sfx-active", true);
bool audio_music_active = config_audio.GetBool("audio-music-active", true);
float audio_master_sfx_volume =
  config_audio.GetFloat("audio-master-sfx-volume", 1.);
float audio_master_music_volume =
  config_audio.GetFloat("audio-master-music-volume", 1.);
int audio_thread_priority;

ConfigureFn(config_audio) {
  AudioSound::init_type();
  AudioGuiFunctor::init_type();

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

  audio_device = new string(config_audio.GetString("audio-device",
                                                   "/dev/dsp"));

  string stmp = config_audio.GetString("audio-thread-priority", "NORMAL");
  for (string::iterator q=stmp.begin(); q!=stmp.end(); ++q)
    (*q) = toupper(*q);
  if (stmp == "LOW")
    audio_thread_priority = 0;
  else if (stmp == "NORMAL")
    audio_thread_priority = 1;
  else if (stmp == "HIGH")
    audio_thread_priority = 2;
  else
    audio_thread_priority = -1;
}

void audio_load_loaders(void) {
  static bool did_load = false;

  if (did_load)
    return;
  Config::ConfigTable::Symbol::iterator i;
  Config::ConfigTable::Symbol loaders;
  config_audio.GetAll("audio-loader", loaders);
  for (i=loaders.begin(); i!=loaders.end(); ++i) {
    Filename dlname = Filename::dso_filename("libaudio_load_" + (*i).Val() +
                                             ".so");
    audio_cat->info() << "loading '" << (*i).Val() << "' loader" << endl;
    void* tmp = load_dso(dlname);
    if (tmp == (void*)0L)
      audio_cat->info() << "unable to load: " << load_dso_error() << endl;
  }
  did_load = true;
}
