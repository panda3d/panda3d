// Filename: audio_manager.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
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
#include "audioManager.h"

#ifdef HAVE_SYS_SOUNDCARD_H
  #include "LinuxAudioManager.h"
#elif defined(HAVE_RAD_MSS)
  #include "MilesAudioManager.h"
#elif defined(WIN32_VC)
  #include "Win32AudioManager.h"
#elif defined(HAVE_MIKMOD)
  #include "MIKMODAudioManager.h"
#else
  #include "NullAudioManager.h"
#endif


// Factory method for getting a platform specific AudioManager:
AudioManager* AudioManager::create_AudioManager() {
  return new NAME_MACRO_FOR_AUDIO_MANAGER();
}


