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


ConfigVariableBool audio_active 
("audio-active", true);

ConfigVariableInt audio_cache_limit 
("audio-cache-limit", 15,
 PRC_DESC("The number of sounds in the cache."));

ConfigVariableDouble audio_volume 
("audio-volume", 1.0f);

ConfigVariableDouble audio_doppler_factor
("audio-doppler-factor", 1.0f);

ConfigVariableDouble audio_distance_factor
("audio-distance-factor", 1.0f);

ConfigVariableDouble audio_drop_off_factor
("audio-drop-off-factor", 1.0f);

ConfigVariableInt audio_min_hw_channels
("audio-min-hw-channels", 15,
 PRC_DESC("Guarantee this many channels on the local sound card, or just "
          "play EVERYTHING in software."));

ConfigVariableBool audio_software_midi 
("audio-software-midi", false);

ConfigVariableFilename audio_dls_file
("audio-dls-file", "");

ConfigVariableBool audio_play_midi 
("audio-play-midi", true);

ConfigVariableBool audio_play_wave 
("audio-play-wave", true);

ConfigVariableBool audio_play_mp3 
("audio-play-mp3", true);

ConfigVariableInt audio_output_rate 
("audio-output-rate", 22050);

ConfigVariableInt audio_output_bits
("audio-output-bits", 16);

ConfigVariableInt audio_output_channels
("audio-output-channels", 2);

ConfigVariableString audio_library_name
("audio-library-name", "miles_audio");


ConfigureFn(config_audio) {
  AudioManager::init_type();
  AudioSound::init_type();
}



