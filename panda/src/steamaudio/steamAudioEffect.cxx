/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioEffect.cxx
 * @author Jackson Sutherland
 */

#include "pandabase.h"

#include "steamAudioEffect.h"

#include <phonon.h>

TypeHandle SteamAudioEffect::_type_handle;

//vars


//functions

/**
*
**/
SteamAudioEffect::SteamAudioEffect() :
  _isActive(true)
{

}

/**
*
**/
SteamAudioEffect::~SteamAudioEffect() {

}

/**
* If we're not active, we just skip this effect.
**/
void SteamAudioEffect::
set_active(bool val) {
  _isActive = val;
}

/**
*Retrieves the _is_active flag.
**/
bool SteamAudioEffect::
get_active() {
  return _isActive;
}

/**
*returns a blank outBuffer. This shouldn't be called, though.
**/
IPLAudioBuffer SteamAudioEffect::
apply_effect(SteamMovieAudioCursor::SteamGlobalHolder *globals, IPLAudioBuffer inBuffer) {
  IPLAudioBuffer outBuffer;
  iplAudioBufferAllocate(*globals->_steam_context, globals->_channels, globals->_samples, &outBuffer);//Be sure to deallocate this in SteamMovieAudioCursor
  return outBuffer;
}
