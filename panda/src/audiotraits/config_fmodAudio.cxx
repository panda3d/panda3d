// Filename: config_fmodAudio.cxx
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
#ifdef HAVE_FMOD //[


// Please remove this as part of updating fmod:
#error The fmod audio needs repair by the fmod implementers




#include "config_fmodAudio.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "dconfig.h"

ConfigureDef(config_fmodAudio);
NotifyCategoryDef(fmodAudio, ":audio");

ConfigureFn(config_fmodAudio) {
  init_libFmodAudio();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libFmodAudio
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libFmodAudio() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  
  initialized = true;

  AudioManager::register_AudioManager_creator(Create_AudioManager);
}

#endif //]
