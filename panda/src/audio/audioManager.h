// Filename: audioManager.h
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
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

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "config_audio.h"
#include "audioSound.h"

typedef PT(AudioManager) Create_AudioManager_proc();


class EXPCL_PANDA AudioManager : public TypedReferenceCount {
PUBLISHED:
  // Create an AudioManager for each category of sounds you have.
  // E.g.
  //   MySoundEffects = create_AudioManager::AudioManager();
  //   MyMusicManager = create_AudioManager::AudioManager();
  //   ...
  //   my_sound = MySoundEffects.get_sound("neatSfx.mp3");
  //   my_music = MyMusicManager.get_sound("introTheme.mid");

  static PT(AudioManager) create_AudioManager();
  virtual ~AudioManager();

  virtual void shutdown();
  
  // If you're interested in knowing whether this audio manager
  // is valid, here's the call to do it.  It is not necessary
  // to check whether the audio manager is valid before making other
  // calls.  You are free to use an invalid sound manager, you
  // may get silent sounds from it though.  The sound manager and
  // the sounds it creates should not crash the application even
  // when the objects are not valid.
  virtual bool is_valid() = 0;
  
  // Get a sound:
  virtual PT(AudioSound) get_sound(const string& file_name, bool positional = false) = 0;
  PT(AudioSound) get_null_sound();

  // Tell the AudioManager there is no need to keep this one cached.
  // This doesn't break any connection between AudioSounds that have
  // already given by get_sound() from this manager.  It's
  // only affecting whether the AudioManager keeps a copy of the sound
  // in its pool/cache.
  virtual void uncache_sound(const string& file_name) = 0;
  virtual void clear_cache() = 0;
  virtual void set_cache_limit(unsigned int count) = 0;
  virtual unsigned int get_cache_limit() const = 0;

  // Control volume:
  // FYI:
  //   If you start a sound with the volume off and turn the volume 
  //   up later, you'll hear the sound playing at that late point.
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  virtual void set_volume(float volume) = 0;
  virtual float get_volume() const = 0;
  
  // Turn the manager on or off.
  // If you play a sound while the manager is inactive, it won't start.
  // If you deactivate the manager while sounds are playing, they'll
  // stop.
  // If you activate the manager while looping sounds are playing
  // (those that have a loop_count of zero),
  // they will start playing from the begining of their loop.
  // inits to true.
  virtual void set_active(bool flag) = 0;
  virtual bool get_active() const = 0;
  
  // This controls the number of sounds that you allow at once.  This
  // is more of a user choice -- it avoids talk over and the creation
  // of a cacophony.
  // It can also be used to help performance.
  // 0 == unlimited.
  // 1 == mutually exclusive (one sound at a time).  Which is an example of:
  // n == allow n sounds to be playing at the same time.
  virtual void set_concurrent_sound_limit(unsigned int limit = 0) = 0;
  virtual unsigned int get_concurrent_sound_limit() const = 0;
  
  // This is likely to be a utility function for the concurrent_sound_limit
  // options.  It is exposed as an API, because it's reasonable that it
  // may be useful to be here.  It reduces the number of concurrently
  // playing sounds to count by some implementation specific means.
  // If the number of sounds currently playing is at or below count then
  // there is no effect.
  virtual void reduce_sounds_playing_to(unsigned int count) = 0;

  // Stop playback on all sounds managed by this manager.
  // This is effectively the same as reduce_sounds_playing_to(0), but
  // this call may be for efficient on some implementations.
  virtual void stop_all_sounds() = 0;


  // Changes to the positions of 3D spacialized sounds and the listener
  // are all made at once when this method is called. It should be put
  // in the main program loop.
  virtual void audio_3d_update();

  // This controls the "set of ears" that listens to 3D spacialized sound
  // px, py, pz are position coordinates. 
  // vx, vy, vz are a velocity vector in UNITS PER SECOND (default: meters). 
  // fx, fy and fz are the respective components of a unit forward-vector
  // ux, uy and uz are the respective components of a unit up-vector
  // These changes will NOT be invoked until audio_3d_update() is called.
  virtual void audio_3d_set_listener_attributes(float px, float py, float pz,
                                                float vx, float vy, float vz,
                                                float fx, float fy, float fz,
                                                float ux, float uy, float uz);
  virtual void audio_3d_get_listener_attributes(float *px, float *py, float *pz,
                                                float *vx, float *vy, float *vz,
                                                float *fx, float *fy, float *fz,
                                                float *ux, float *uy, float *uz);
 
  // Control the "relative distance factor" for 3D spacialized audio. Default is 1.0
  // Fmod uses meters internally, so give a float in Units-per meter
  // Don't know what Miles uses.
  virtual void audio_3d_set_distance_factor(float factor);
  virtual float audio_3d_get_distance_factor() const;

  // Control the presence of the Doppler effect. Default is 1.0
  // Exaggerated Doppler, use >1.0
  // Diminshed Doppler, use <1.0
  virtual void audio_3d_set_doppler_factor(float factor);
  virtual float audio_3d_get_doppler_factor() const;

  // Exaggerate or diminish the effect of distance on sound. Default is 1.0
  // Faster drop off, use >1.0
  // Slower drop off, use <1.0
  virtual void audio_3d_set_drop_off_factor(float factor);
  virtual float audio_3d_get_drop_off_factor() const;

public:
  static void register_AudioManager_creator(Create_AudioManager_proc* proc);

protected:
  friend class AudioSound;
  
  // Avoid adding data members (instance variables) to this mostly abstract
  // base class.  This allows implementors of various sound systems the
  // best flexibility.
  
  static Create_AudioManager_proc* _create_AudioManager;
  PT(AudioSound) _null_sound;

  AudioManager() {
    // intentionally blank.
  }


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioManager",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif /* __AUDIO_MANAGER_H__ */
