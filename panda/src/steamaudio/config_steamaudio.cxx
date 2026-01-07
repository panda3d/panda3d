/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_steamaudio.cxx
 * @author Jackson Sutherland
 */

#include "config_steamaudio.h"

#include "steamMovieAudio.h"
#include "steamMovieAudioCursor.h"

#include "steamAudioEffect.h"
#include "steamDirectEffect.h"

#include "pandaSystem.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_STEAM_AUDIO)
#error Buildsystem error: BUILDING_STEAM_AUDIO not defined
#endif

ConfigureDef(config_steamaudio);
NotifyCategoryDef(steamaudio, ":audio");

ConfigureFn(config_steamaudio) {
  init_libsteamaudio();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libsteamaudio() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  SteamMovieAudio::init_type();
  SteamMovieAudioCursor::init_type();

  SteamAudioEffect::init_type();
  SteamDirectEffect::init_type();

  PandaSystem* ps = PandaSystem::get_global_ptr();
  ps->add_system("SteamAudio");
  //TODO:: Do we need to set this as a subsystem of Audio?
}
