// Filename: audioManager.h
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

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "config_audio.h"
#include "audioSound.h"

typedef PT(AudioManager) Create_AudioManager_proc();


class EXPCL_PANDA AudioManager : public ReferenceCount {
PUBLISHED:
  // Create an AudioManager for each category of sounds you have.
  // E.g.
  //   MySoundEffects = create_AudioManager::AudioManager();
  //   MyMusicManager = create_AudioManager::AudioManager();
  //   ...
  //   my_sound = MySoundEffects.get_sound("neatSfx.mp3");
  //   my_music = MyMusicManager.get_sound("introTheme.mid");

  static PT(AudioManager) create_AudioManager();
  virtual ~AudioManager() {}
  virtual bool is_valid() = 0;
  
  // Get a sound:
  virtual PT(AudioSound) get_sound(const string& file_name) = 0;
  PT(AudioSound) get_null_sound();

  // Tell the AudioManager there is no need to keep this one cached.
  // This doesn't break any connection between AudioSounds that have
  // already given by get_sound() from this manager.  It's
  // only affecting whether the AudioManager keeps a copy of the sound
  // in its pool/cache.
  virtual void uncache_sound(const string& file_name) = 0;
  virtual void clear_cache() = 0;
  virtual void set_cache_limit(int count) = 0;
  virtual int get_cache_limit() = 0;

  // if set, turn off any currently-playing sounds before playing
  // a new one (useful for midi songs)
  void set_mutually_exclusive(bool bExclusive);

  // Control volume:
  // FYI:
  //   If you start a sound with the volume off and turn the volume 
  //   up later, you'll hear the sound playing at that late point.
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  virtual void set_volume(float volume) = 0;
  virtual float get_volume() = 0;
  
  // Turn the manager on or off.
  // If you play a sound while the manager is inactive, it won't start.
  // If you deactivate the manager while sounds are playing, they'll
  // stop.
  // If you activate the manager while looping sounds are playing
  // (those that have a loop_count of zero),
  // they will start playing from the begining of their loop.
  // inits to true.
  virtual void set_active(bool flag) = 0;
  virtual bool get_active() = 0;

public:
  static void register_AudioManager_creator(Create_AudioManager_proc* proc);

protected:
  static Create_AudioManager_proc* _create_AudioManager;
  bool _bExclusive;
  PT(AudioSound) _null_sound;

  AudioManager() {
    // intentionally blank.
  }
};

#endif /* __AUDIO_MANAGER_H__ */
