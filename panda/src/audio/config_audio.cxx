// Filename: config_audio.cxx
// Created by:  cary (22Sep00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_audio.h"
#include "dconfig.h"
#include "audioManager.h"
#include "audioSound.h"

Configure(config_audio);
NotifyCategoryDef(audio, "");


bool audio_active 
    =config_audio.GetBool("audio-active", true);

//number of sounds in cache
int audio_cache_limit 
    =config_audio.GetInt("audio-cache-limit", 15);  

float audio_volume 
    =config_audio.GetFloat("audio-volume", 1.0f);

bool audio_software_midi 
    =config_audio.GetBool("audio-software-midi", false);

bool audio_play_midi 
    =config_audio.GetBool("audio-play-midi", true);

bool audio_play_wave 
    =config_audio.GetBool("audio-play-wave", true);

bool audio_play_mp3 
    =config_audio.GetBool("audio-play-mp3", true);

int audio_output_rate 
    =config_audio.GetInt("audio-output-rate", 22050);

int audio_output_bits
    =config_audio.GetInt("audio-output-bits", 16);

int audio_output_channels
    =config_audio.GetInt("audio-output-channels", 2);

string* audio_dls_file;

string* audio_library_name;


ConfigureFn(config_audio) {
  audio_dls_file = new string(
      config_audio.GetString("audio-dls-file", ""));

  audio_library_name = new string(
      config_audio.GetString("audio-library-name", "miles_audio"));

  AudioManager::init_type();
  AudioSound::init_type();
}



