// Filename: config_milesAudio.cxx
// Created by:  skyler
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "config_milesAudio.h"
#include "milesAudioManager.h"
#include "milesAudioSound.h"
#include "milesAudioSample.h"
#include "milesAudioSequence.h"
#include "milesAudioStream.h"
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
          "this value will be expanded.  This only applies to files "
          "within the miles-audio-preload-threshold."));

ConfigVariableInt miles_audio_preload_threshold
("miles-audio-preload-threshold", -1,
 PRC_DESC("This should be no smaller "
          "than miles-audio-expand-mp3-threshold.  Files that are smaller "
          "than this number of bytes will be preloaded and kept "
          "resident in memory, while files that are this size or larger "
          "will be streamed from disk.  Set this to -1 to preload "
          "every file."));

ConfigVariableBool miles_audio_panda_threads
("miles-audio-panda-threads", true,
 PRC_DESC("Set this true to service Miles background audio via Panda's "
          "threading interface, instead of Miles' built-in threading "
          "interface.  This gives Panda more control over the threading, "
          "and ensures better lock protection within Panda.  This has "
          "no meaning unless Panda is compiled with thread support."));

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
  MilesAudioManager::init_type();
  MilesAudioSound::init_type();
  MilesAudioSample::init_type();
  MilesAudioSequence::init_type();
  MilesAudioStream::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("Miles");
  ps->add_system("audio");
  ps->set_system_tag("audio", "implementation", "Miles");
}

////////////////////////////////////////////////////////////////////
//     Function: get_audio_manager_func_miles_audio
//  Description: This function is called when the dynamic library is
//               loaded; it should return the Create_AudioManager
//               function appropriate to create a MilesAudioManager.
///////////////////////////////////////////////////////////////////
Create_AudioManager_proc *
get_audio_manager_func_miles_audio() {
  init_libMilesAudio();
  return &Create_MilesAudioManager;
}

#endif //]
