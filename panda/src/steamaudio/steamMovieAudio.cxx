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

#include "steamMovieAudioCursor.h"

SteamMovieAudio::
SteamMovieAudio(const std::string& name, PT(MovieAudio) audio_source, NodePath source, NodePath listener) :
  MovieAudio(name),
  _sourceNP(source),
  _listenerNP(listener),
  _audio_source(audio_source)
{
  //SteamAudio initialization
  if (_steamContext == nullptr) {//we haven't made a context yet
    IPLContextSettings contextSettings{};
    contextSettings.version = STEAMAUDIO_VERSION;
    iplContextCreate(&contextSettings, _steamContext);
  }
}

SteamMovieAudio::
~SteamMovieAudio() {

}

PT(MovieAudioCursor) SteamMovieAudio::
open() {
  return new SteamMovieAudioCursor(this);
}

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
  if (index < _steam_effects.size()) {
    return *_steam_effects[index];
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
get(const Filename& name, NodePath source, NodePath listener) {
  return get(MovieAudio().get(name), source, listener);
}

/**
*Creates a SteamMovieAudio based on an existing MovieAudio.
*/
PT(SteamMovieAudio) SteamMovieAudio::
get(const PT(MovieAudio) audio, NodePath source, NodePath listener) {
  PT(SteamMovieAudio) newSteamAudio = new SteamMovieAudio(*audio.get_name().append(std::string(": Steam Audio")), audio, source, listener);
  return newSteamAudio;
}

/**
*Steam Audio has different different axes, so this function quickly translates them.
*/
void SteamMovieAudio::
sa_coordinate_transform(float x1, float y1, float z1, IPLVector3& vals) {
  vals.x = x1;
  vals.y = z1;
  vals.z = -y1;
}

void SteamMovieAudio::
get_listener_position(IPLVector3& vals) {
  sa_coordinate_transform(_listenerNP.get_x(), _listenerNP.get_y(), _listenerNP.get_z(), vals);
}

void SteamMovieAudio::
get_source_position(IPLVector3& vals) {
  sa_coordinate_transform(_sourceNP.get_x(), _sourceNP.get_y(), _sourceNP.get_z(), vals);
}

void SteamMovieAudio::
get_source_coordinates(IPLCoordinateSpace3& vals) {
  LVecBase3f temp = _sourceNP.get_quat().get_forward();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.ahead);

  temp = _sourceNP.get_quat().get_right();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.right);

  temp = _sourceNP.get_quat().get_up();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.up);

  temp = _sourceNP.get_pos();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.origin);
}

void SteamMovieAudio::
get_Listener_coordinates(IPLCoordinateSpace3& vals) {
  LVecBase3f temp = _listenerNP.get_quat().get_forward();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.ahead);

  temp = _listenerNP.get_quat().get_right();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.right);

  temp = _listenerNP.get_quat().get_up();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.up);

  temp = _listenerNP.get_pos();
  sa_coordinate_transform(temp.get_x(), temp.get_y(), temp.get_z(), vals.origin);
}
