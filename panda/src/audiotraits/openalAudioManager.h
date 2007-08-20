// Filename: openalAudioManager.h
// Created by:  Ben Buchwald <bb2@alumni.cmu.edu>
//
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


#ifndef __OPENAL_AUDIO_MANAGER_H__
#define __OPENAL_AUDIO_MANAGER_H__

#include "pandabase.h"
#ifdef HAVE_OPENAL //[

#include "audioManager.h"
#include "pset.h"
#include "pmap.h"
#include "pdeque.h"

//The Includes needed for OpenAL
#include <al.h>
#include <alc.h>
#include <alut.h>

class OpenALAudioSound;

extern void al_audio_errcheck(const char *context);
extern void alc_audio_errcheck(const char *context,ALCdevice* device);
extern void alut_audio_errcheck(const char *context);

class EXPCL_OPENAL_AUDIO OpenALAudioManager : public AudioManager {
  class SoundData;
  
  friend class OpenALAudioSound;
  friend class OpenALSoundData;
 public:

  //Constructor and Destructor
  OpenALAudioManager();
  virtual ~OpenALAudioManager();

  virtual void shutdown();

  virtual bool is_valid();
          
  virtual PT(AudioSound) get_sound(const string&, bool positional = false);
  
  virtual void uncache_sound(const string&);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;
    
  virtual void set_volume(float);
  virtual float get_volume() const;
          
  void set_play_rate(float play_rate);
  float get_play_rate() const;

  virtual void set_active(bool);
  virtual bool get_active() const;

  // This controls the "set of ears" that listens to 3D spacialized sound
  // px, py, pz are position coordinates. Can be 0.0f to ignore.
  // vx, vy, vz are a velocity vector in UNITS PER SECOND.
  // fx, fy and fz are the respective components of a unit forward-vector
  // ux, uy and uz are the respective components of a unit up-vector
  // These changes will NOT be invoked until audio_3d_update() is called.
  virtual void audio_3d_set_listener_attributes(float px, float py, float pz,
                                                float vx, float xy, float xz, 
                                                float fx, float fy, float fz,
                                                float ux, float uy, float uz);

  virtual void audio_3d_get_listener_attributes(float *px, float *py, float *pz,
                                                float *vx, float *vy, float *vz,
                                                float *fx, float *fy, float *fz,
                                                float *ux, float *uy, float *uz);
          
  // Control the "relative distance factor" for 3D spacialized audio in units-per-foot. Default is 1.0
  // OpenAL has no distance factor but we use this as a scale
  // on the min/max distances of sounds to preserve FMOD compatibility.
  // Also, adjusts the speed of sound to compensate for unit difference.
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

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;
  virtual void reduce_sounds_playing_to(unsigned int count);

  virtual void stop_all_sounds();

  virtual void update();

private:
  void make_current() const;

  PT(SoundData) load(Filename file_name);

  // Tell the manager that the sound dtor was called.
  void release_sound(OpenALAudioSound* audioSound);
  
  void most_recently_used(const string& path);
  void uncache_a_sound();

  void starting_sound(OpenALAudioSound* audio);
  void stopping_sound(OpenALAudioSound* audio);

  void cleanup();
private:

  // The sound cache:
  class SoundData : public ReferenceCount {
  public:
    SoundData(OpenALAudioManager* manager);
    ~SoundData();
    float get_length();

    OpenALAudioManager* _manager;
    string _basename;
    ALuint _buffer;
    bool _has_length;
    float _length;  // in seconds.
  };
  typedef pmap<string, PT(SoundData) > SoundMap;
  SoundMap _sounds;
  int _cache_limit;

  typedef pset<OpenALAudioSound* > AudioSet;
  // The offspring of this manager:
  AudioSet _sounds_on_loan;

  typedef pset<OpenALAudioSound* > SoundsPlaying;
  // The sounds from this manager that are currently playing
  SoundsPlaying _sounds_playing;

  // The Least Recently Used mechanism:
  typedef pdeque<const string* > LRU;
  LRU _lru;

  // State:
  float _volume;
  float _play_rate;
  bool _active;
  bool _cleanup_required;
  // keep a count for startup and shutdown:
  static int _active_managers;
  static bool _openal_active;
  unsigned int _concurrent_sound_limit;
  
  bool _is_valid;
  
  typedef pset<OpenALAudioManager *> Managers;
  static Managers *_managers;

  static ALCdevice* _device; 
  static ALCcontext* _context;

  // cache of openal sources, use only for playing sounds
  typedef pset<ALuint > SourceCache;
  static SourceCache *_al_sources;

  float _distance_factor;
  float _doppler_factor;
  float _drop_off_factor;

  ALfloat _position[3];
  ALfloat _velocity[3];
  ALfloat _forward_up[6];

  ////////////////////////////////////////////////////////////
  //These are needed for Panda's Pointer System. DO NOT ERASE!
  ////////////////////////////////////////////////////////////

 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "OpenALAudioManager", AudioManager::get_class_type());
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

  ////////////////////////////////////////////////////////////
  //DONE
  ////////////////////////////////////////////////////////////

};

EXPCL_OPENAL_AUDIO PT(AudioManager) Create_AudioManager();


#endif //]

#endif /* __OPENAL_AUDIO_MANAGER_H__ */
