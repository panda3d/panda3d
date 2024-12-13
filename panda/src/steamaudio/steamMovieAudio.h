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
#include "nodePath.h"

#include <phonon.h>//Import steam audio

class MovieAudioCursor;
class SteamAudioCursor;
class SteamAudioEffect;


/***
* This class points to a normal MovieAudio, which is then run through Steam Audio's DSP.
*/
class EXPCL_STEAM_AUDIO SteamMovieAudio : public MovieAudio {
PUBLISHED:
  explicit SteamMovieAudio(const std::string& name = "Empyt SteamMovieAudio", MovieAudio& audio_source);
  virtual ~SteamMovieAudio();
  virtual PT(MovieAudioCursor) open();

  int add_steam_audio_effect(SteamAudioEffect effect);
  int find_steam_audio_effect(SteamAudioEffect effect);
  SteamAudioEffect get_steam_audio_effect(int index);
  bool remove_steam_audio_effect(int index);
  bool remove_steam_audio_effect(SteamAudioEffect effect);

  static PT(SteamMovieAudio) get(const Filename& name);
  static PT(SteamMovieAudio) get(const MovieAudio& audio);

private:

  static IPLContext* _steamContext;

  typedef pvector<PT(SteamAudioEffect)> SAEffects;
  SAEffects _steam_effects;

  MovieAudio* _audio_source;  

  friend class SteamMovieAudioCursor;

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
