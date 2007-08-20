// Filename: config_openalAudio.cxx
// Created by:  cort
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
#ifdef HAVE_OPENAL //[


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

  AudioManager::register_AudioManager_creator(Create_AudioManager);

  OpenALAudioManager::init_type();
  OpenALAudioSound::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenAL");
  ps->add_system("audio");
  ps->set_system_tag("audio", "implementation", "OpenAL");
}

#endif //]
