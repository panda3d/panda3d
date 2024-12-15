/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamMovieAudio.h
 * @author Jackson Sutherland
 */

#ifndef STEAMAUDIOMANAGER_H
#define STEAMAUDIOMANAGER_H

#include "pandabase.h"

#include "movieAudio.h"
#include "movieAudioCursor.h"
#include "nodePath.h"

#include <phonon.h>//Import steam audio

class SteamMovieAudioCursor;
class SteamAudioEffect;


/***
* This class points to a normal MovieAudio, which is then run through Steam Audio's DSP.
*/
class EXPCL_STEAM_AUDIO SteamMovieAudio : public MovieAudio {
PUBLISHED:
  explicit SteamMovieAudio(const std::string& name, MovieAudio& audio_source, NodePath source, NodePath listener);
  virtual ~SteamMovieAudio();
  virtual PT(MovieAudioCursor) open();

  int add_steam_audio_effect(SteamAudioEffect effect);
  int find_steam_audio_effect(SteamAudioEffect effect);
  SteamAudioEffect get_steam_audio_effect(int index);
  bool remove_steam_audio_effect(int index);
  bool remove_steam_audio_effect(SteamAudioEffect effect);

  static PT(SteamMovieAudio) get(const Filename& name, NodePath source, NodePath listener);
  static PT(SteamMovieAudio) get(const MovieAudio& audio, NodePath source, NodePath listener);

private:

  static PT(IPLContext) _steamContext;

  typedef pvector<PT(SteamAudioEffect)> SAEffects;
  SAEffects _steam_effects;

  PT(MovieAudio) _audio_source;

  NodePath _listenerNP;
  NodePath _sourceNP;

  void sa_coordinate_transform(float x1, float y1, float z1, IPLVector3& vals);

  friend class SteamMovieAudioCursor;

public:

  void get_listener_position(IPLVector3& vals);
  void get_source_position(IPLVector3& vals);
  void get_source_coordinates(IPLCoordinateSpace3& vals);
  void get_Listener_coordinates(IPLCoordinateSpace3& vals);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "SteamMovieAudio", MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};


#endif
