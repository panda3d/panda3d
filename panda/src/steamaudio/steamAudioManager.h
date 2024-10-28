/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioManager.h
 * @author Jackson Sutherland
 *
 * Hello! I wanted to note that this at least started out as essentially
 * a copy-and-paste modification to openalAudioManager. It was tempting
 * to try and come up with a solution that didn't involve making a new
 * AudioManager class,or a solution that couldeasily work with both FMOD
 * and OpenAl.However; at least in OpenAL's case; updating audio effects
 * on a sound while it's playing requires more or less chaining audio buffers
 * together, something that does not appear to be possible with the current openalAudioManager.
 */


#ifndef STEAMAUDIOMANAGER_H
#define STEAMAUDIOMANAGER_H

#include "pandabase.h"

#include "openalAudioManager.h"
#include "plist.h"
#include "pmap.h"
#include "pset.h"
#include "movieAudioCursor.h"
#include "reMutex.h"

#include <phonon.h>//Import steam audio

 // OSX uses the OpenAL framework
#ifdef HAVE_OPENAL_FRAMEWORK
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

class EXPCL_STEAMAUDIO SteamAudioManager final : public OpenALAudioManager {

PUBLISHED:
  SteamAudioManager();
  virtual ~SteamAudioManager();

  virtual void shutdown();

  virtual bool is_valid();

  virtual PT(SteamAudioSound) get_sound(const Filename&, bool positional = false, int mode = SM_stream);//change in default value from OpenALSoundManager,
  virtual PT(SteamAudioSound) get_sound(MovieAudio* sound, bool positional = false, int mode = SM_stream);//This is because streaming is nessicary for real-time updating of effects.

  virtual void uncache_sound(const Filename&);
  virtual void clear_cache();

  virtual void update();

private:
  std::string select_audio_device();

  bool can_use_audio(MovieAudioCursor* source);
  bool should_load_audio(MovieAudioCursor* source, int mode);

  SoundData* get_sound_data(MovieAudio* source, int mode);

  // Tell the manager that the sound dtor was called.
  void release_sound(OpenALAudioSound* audioSound);
  void discard_excess_cache(int limit);

  void delete_buffer(ALuint buffer);

  void starting_sound(OpenALAudioSound* audio);
  void stopping_sound(OpenALAudioSound* audio);

  void cleanup();

private:
  // This global lock protects all access to SteamAudio library interfaces.
  static ReMutex _steamLock;

  // These are needed for Panda's Pointer System.  DO NOT ERASE!

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "SteamAudioManager", AudioManager::get_class_type());
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

  // DONE

};

#endif
