/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamMovieAudio.cxx
 * @author Jackson Sutherland
 */


#include "pandabase.h"

#include "steamMovieAudio.h"

//Steam Audio
#include <phonon.h>

SteamMovieAudio::
explicit SteamMovieAudio(const std::string& name = "Blank Audio", MovieAudio& audio_source) :
  MovieAudio(name)
{
  //SteamAudio initialization
  if (_steamContext == nullptr) {//we haven't made a context yet
    IPLContextSettings contextSettings{};
    contextSettings.version = STEAMAUDIO_VERSION;
    iplContextCreate(&contextSettings, _steamContext);
  }

  
  _audio_source = &audio_source;
}

SteamMovieAudio::
~SteamMovieAudio()
{

}

//Add open here:


/**
*returns the index of the newly-added steam audio effect.
*/
int SteamMovieAudio::
add_steam_audio_effect(SteamAudioEffect effect) {
  _steam_effects.push_back(&effect);
  pvector<PT(SteamAudioEffect)>::iterator i = std::find(_steam_effects.begin(), _steam_effects.end(), &effect);
  return i - _steam_effects.begin();
}

/**
*Returns the index of a SteamAudioEffect, or -1 if not found.
*/
int SteamMovieAudio::
find_steam_audio_effect(SteamAudioEffect effect) {
  auto i = std::find(_steam_effects.begin(), _steam_effects.end(), &effect);
  if (i != _steam_effects.end()) {
    return i - _steam_effects.begin();
  }
  else {
    return -1;
  }

}

/**
*Returns the SteamAudioEffect at *index*, else returns a null effect.
*/
SteamAudioEffect SteamMovieAudio::
get_steam_audio_effect(int index) {
  if (index < myVector.size()) {
    return _steam_effects[index];
  }
  else {
    return SteamAudioEffect();
  }
}

/**
*Removes an effect from this object, then returns true if successful.
*/
bool SteamMovieAudio::
remove_steam_audio_effect(int index) {
  if (!_steam_effects.empty()) {
    _steam_effects.erase(_steam_effects.begin() + index);
    return true;
  }
  else {
    return false;
  }
}

bool SteamMovieAudio::
remove_steam_audio_effect(SteamAudioEffect effect) {
  auto it = std::find(_steam_effects.begin(), _steam_effects.end(), effect);
  if (!_steam_effects.empty()) {
    _steam_effects.erase(it);
    return true;
  }
  else {
    return false;
  }
}

PT(SteamMovieAudio) SteamMovieAudio::
get(const Filename& name) {
  return SteamMovieAudio.get(MovieAudio.get(name));
}

/**
*Creates a SteamMovieAudio based on an existing MovieAudio.
*/
PT(SteamMovieAudio) SteamMovieAudio::
get(const MovieAudio& audio) {
  SteamMovieAudio newSteamAudio = SteamMovieAudio(audio.get_name().append(": Steam Audio"), audio);
  return newSteamAudio;
}
//NodePath& source, NodePath& listener
