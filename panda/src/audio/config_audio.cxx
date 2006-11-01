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
#include "audioLoadRequest.h"
#include "audioManager.h"
#include "audioSound.h"
#include "nullAudioDSP.h"
#include "nullAudioManager.h"
#include "nullAudioSound.h"

Configure(config_audio);
NotifyCategoryDef(audio, "");

ConfigVariableBool audio_active 
("audio-active", true);

ConfigVariableInt audio_cache_limit 
("audio-cache-limit", 15,
 PRC_DESC("The number of sounds in the cache."));

ConfigVariableString audio_library_name
("audio-library-name", "miles_audio");

ConfigVariableDouble audio_volume 
("audio-volume", 1.0f);

ConfigVariableFilename audio_dls_file 
("audio-dls-file", Filename(),
 PRC_DESC("Specifies a DLS file that defines an instrument set to load "
          "for MIDI file playback.  If this is not specified, the sound "
          "interface will try to use the system default DLS file, if "
          "one is available; the likely success of this depends on the "
          "operating system."));

// Config variables for Fmod:

//I should note this somewhere.  The actual number of sound one could
//play is 2 times whatever the default is here.  In this case 32 * 2 =
//64.  The reason for this, is because of the way Panda creates two
//seperate audio managers.  One for sound effects, the other for music
//files.  At one time this used to be a concern, because wave files
//[WAV, AIF, MP3, etc...]  and Music files [MID MOD IT] used to be
//treated differently.  However in current Audio APIs [particularly
//FMOD] a sound file is treated the same no matter what it is.

ConfigVariableInt fmod_number_of_sound_channels
("fmod-number-of-sound-channels", 32,
 PRC_DESC("Guarantee this many channels you will have with FMOD.  AKA the max number of sounds you can play at one time.") );

ConfigVariableBool fmod_use_surround_sound
("fmod-use-surround-sound", false, 
 PRC_DESC("Determines if an FMOD Flavor of PANDA use 5.1 Surround Sound or Not.") );


// Config variables for Miles:

ConfigVariableBool audio_software_midi 
("audio-software-midi", true);

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



ConfigureFn(config_audio) {
  AudioLoadRequest::init_type();
  AudioManager::init_type();
  AudioSound::init_type();
  AudioDSP::init_type();
  nullAudioDSP::init_type();
  NullAudioManager::init_type();
  NullAudioSound::init_type();
}



