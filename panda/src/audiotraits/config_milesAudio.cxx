// Filename: config_milesAudio.cxx
// Created by:  skyler
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

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "config_milesAudio.h"
#include "milesAudioManager.h"
#include "milesAudioSound.h"
#include "pandaSystem.h"
#include "dconfig.h"

ConfigureDef(config_milesAudio);
NotifyCategoryDef(milesAudio, ":audio");

ConfigureFn(config_milesAudio) {
  init_libMilesAudio();
}

ConfigVariableBool miles_audio_force_midi_reset
("audio-force-midi-reset", true);

ConfigVariableInt miles_audio_expand_mp3_threshold
("miles-audio-expand-mp3-threshold", 16384,
 PRC_DESC("This enables a Miles workaround in which small MP3 files are "
          "expanded in-memory at load time into WAV format, which can "
          "work around problems with Miles being unable to correctly "
          "report the length of, or seek within, a variable bit-rate encoded "
          "MP3 file.  Any MP3 file whose length in bytes is less than "
          "this value will be expanded."));

ConfigVariableInt miles_audio_calc_mp3_threshold
("miles-audio-calc-mp3-threshold", 1048576,
 PRC_DESC("This is a second fallback for miles-audio-expand-mp3-threshold.  "
          "Any MP3 file whose length in bytes is less than this value "
          "will have its length calculated on demand, by running through "
          "the entire file first.  This works around a Miles bug in "
          "which variable bit-rate encoded MP3 files do not report an "
          "accurate length."));

////////////////////////////////////////////////////////////////////
//     Function: init_libMilesAudio
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libMilesAudio() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  AudioManager::register_AudioManager_creator(Create_AudioManager);

  MilesAudioManager::init_type();
  MilesAudioSound::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("Miles");
  ps->add_system("audio");
  ps->set_system_tag("audio", "implementation", "Miles");
}

#endif //]
