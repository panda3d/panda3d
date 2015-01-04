// Filename: config_openalAudio.cxx
// Created by:  cort
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

#include "config_openalAudio.h"
#include "openalAudioManager.h"
#include "openalAudioSound.h"
#include "pandaSystem.h"
#include "dconfig.h"

ConfigureDef(config_openalAudio);
NotifyCategoryDef(openalAudio, ":audio");

ConfigureFn(config_openalAudio) {
  init_libOpenALAudio();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libOpenALAudio
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libOpenALAudio() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  
  initialized = true;
  OpenALAudioManager::init_type();
  OpenALAudioSound::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenAL");
  ps->add_system("audio");
  ps->set_system_tag("audio", "implementation", "OpenAL");
}

////////////////////////////////////////////////////////////////////
//     Function: get_audio_manager_func_openal_audio
//  Description: This function is called when the dynamic library is
//               loaded; it should return the Create_AudioManager
//               function appropriate to create an OpenALAudioManager.
///////////////////////////////////////////////////////////////////
Create_AudioManager_proc *
get_audio_manager_func_openal_audio() {
  init_libOpenALAudio();
  return &Create_OpenALAudioManager;
}
