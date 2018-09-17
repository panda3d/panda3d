/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openalAudioManager.h
 * @author Ben Buchwald <bb2@alumni.cmu.edu>
 */

#ifndef __OPENAL_AUDIO_MANAGER_H__
#define __OPENAL_AUDIO_MANAGER_H__

#include "pandabase.h"

#include "audioManager.h"
#include "plist.h"
#include "pmap.h"
#include "pset.h"
#include "movieAudioCursor.h"
#include "reMutex.h"

// OSX uses the OpenAL framework
#ifdef HAVE_OPENAL_FRAMEWORK
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

class OpenALAudioSound;

extern void al_audio_errcheck(const char *context);
extern void alc_audio_errcheck(const char *context,ALCdevice* device);

class EXPCL_OPENAL_AUDIO OpenALAudioManager : public AudioManager {
  class SoundData;

  friend class OpenALAudioSound;
  friend class OpenALSoundData;

 public:
  // Constructor and Destructor
  OpenALAudioManager();
  virtual ~OpenALAudioManager();

  virtual void shutdown();

  virtual bool is_valid();

  virtual PT(AudioSound) get_sound(const Filename &, bool positional = false, int mode=SM_heuristic);
  virtual PT(AudioSound) get_sound(MovieAudio *sound, bool positional = false, int mode=SM_heuristic);

  virtual void uncache_sound(const Filename &);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

  virtual void set_volume(PN_stdfloat);
  virtual PN_stdfloat get_volume() const;

  void set_play_rate(PN_stdfloat play_rate);
  PN_stdfloat get_play_rate() const;

  virtual void set_active(bool);
  virtual bool get_active() const;

  // This controls the "set of ears" that listens to 3D spacialized sound px,
  // py, pz are position coordinates.  Can be 0.0f to ignore.  vx, vy, vz are
  // a velocity vector in UNITS PER SECOND. fx, fy and fz are the respective
  // components of a unit forward-vector ux, uy and uz are the respective
  // components of a unit up-vector These changes will NOT be invoked until
  // audio_3d_update() is called.
  virtual void audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
                                                PN_stdfloat vx, PN_stdfloat xy, PN_stdfloat xz,
                                                PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz,
                                                PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz);

  virtual void audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz,
                                                PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz,
                                                PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz,
                                                PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz);


  // Control the "relative scale that sets the distance factor" units for 3D
  // spacialized audio. This is a float in units-per-meter. Default value is
  // 1.0, which means that Panda units are understood as meters; for e.g.
  // feet, set 3.28. This factor is applied only to Fmod and OpenAL at the
  // moment.
  // OpenAL in fact has no distance factor like Fmod, but works with the speed
  // of sound instead, so we use this factor to scale the speed of sound.
  virtual void audio_3d_set_distance_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_distance_factor() const;

  // Control the presence of the Doppler effect.  Default is 1.0 Exaggerated
  // Doppler, use >1.0 Diminshed Doppler, use <1.0
  virtual void audio_3d_set_doppler_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_doppler_factor() const;

  // Exaggerate or diminish the effect of distance on sound.  Default is 1.0
  // Faster drop off, use >1.0 Slower drop off, use <1.0
  virtual void audio_3d_set_drop_off_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_drop_off_factor() const;

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;
  virtual void reduce_sounds_playing_to(unsigned int count);

  virtual void stop_all_sounds();

  virtual void update();

private:
  std::string select_audio_device();

  void make_current() const;

  bool can_use_audio(MovieAudioCursor *source);
  bool should_load_audio(MovieAudioCursor *source, int mode);

  SoundData *get_sound_data(MovieAudio *source, int mode);

  // Tell the manager that the sound dtor was called.
  void release_sound(OpenALAudioSound* audioSound);
  void increment_client_count(SoundData *sd);
  void decrement_client_count(SoundData *sd);
  void discard_excess_cache(int limit);

  void delete_buffer(ALuint buffer);

  void starting_sound(OpenALAudioSound* audio);
  void stopping_sound(OpenALAudioSound* audio);

  void cleanup();

private:
  // This global lock protects all access to OpenAL library interfaces.
  static ReMutex _lock;

  // An expiration queue is a list of SoundData that are no longer being used.
  // They are kept around for a little while, since it is common to stop using
  // a sound for a brief moment and then quickly resume.

  typedef plist<void *> ExpirationQueue;
  ExpirationQueue _expiring_samples;
  ExpirationQueue _expiring_streams;

/*
 * An AudioSound that uses a SoundData is called a "client" of the SoundData.
 * The SoundData keeps track of how many clients are using it.  When the
 * number of clients drops to zero, the SoundData is no longer in use.  The
 * expiration queue is a list of all SoundData that aren't in use, in least-
 * recently-used order.  If a SoundData in the expiration queue gains a new
 * client, it is removed from the expiration queue.  When the number of sounds
 * in the expiration queue exceeds the cache limit, the first sound in the
 * expiration queue is purged.
 */

  class SoundData {
  public:
    SoundData();
    ~SoundData();
    OpenALAudioManager*  _manager;
    PT(MovieAudio)       _movie;
    ALuint               _sample;
    PT(MovieAudioCursor) _stream;
    double               _length;
    int                  _rate;
    int                  _channels;
    int                  _client_count;
    ExpirationQueue::iterator _expire;
  };


  typedef phash_map<std::string, SoundData *> SampleCache;
  SampleCache _sample_cache;

  typedef phash_set<PT(OpenALAudioSound)> SoundsPlaying;
  SoundsPlaying _sounds_playing;

  typedef phash_set<OpenALAudioSound *> AllSounds;
  AllSounds _all_sounds;

  // State:
  int _cache_limit;
  PN_stdfloat _volume;
  PN_stdfloat _play_rate;
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

  PN_stdfloat _distance_factor;
  PN_stdfloat _doppler_factor;
  PN_stdfloat _drop_off_factor;

  ALfloat _position[3];
  ALfloat _velocity[3];
  ALfloat _forward_up[6];

  // These are needed for Panda's Pointer System.  DO NOT ERASE!

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

  // DONE

};

EXPCL_OPENAL_AUDIO AudioManager *Create_OpenALAudioManager();

#endif /* __OPENAL_AUDIO_MANAGER_H__ */
