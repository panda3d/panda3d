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

#include "audioManager.h"
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

class EXPCL_STEAMAUDIO SteamAudioManager final : public AudioManager {

public:
  SteamAudioManager();
  virtual ~SteamAudioManager();

  virtual void shutdown();

  virtual bool is_valid();

  virtual PT(AudioSound) get_sound(const Filename&, bool positional = false, int mode = SM_heuristic);
  virtual PT(AudioSound) get_sound(MovieAudio* sound, bool positional = false, int mode = SM_heuristic);

  virtual void uncache_sound(const Filename&);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

  virtual void set_volume(PN_stdfloat);
  virtual PN_stdfloat get_volume() const;

  void set_play_rate(PN_stdfloat play_rate);
  PN_stdfloat get_play_rate() const;

  virtual void set_active(bool);
  virtual bool get_active() const;

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;
  virtual void reduce_sounds_playing_to(unsigned int count);

  virtual void stop_all_sounds();

  virtual void update();

private:
  std::string select_audio_device();

  void make_current() const;

  bool can_use_audio(MovieAudioCursor* source);
  bool should_load_audio(MovieAudioCursor* source, int mode);

  SoundData* get_sound_data(MovieAudio* source, int mode);

  // Tell the manager that the sound dtor was called.
  void release_sound(OpenALAudioSound* audioSound);
  void increment_client_count(SoundData* sd);
  void decrement_client_count(SoundData* sd);
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

  typedef plist<void*> ExpirationQueue;
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
    OpenALAudioManager* _manager;
    PT(MovieAudio)       _movie;
    ALuint               _sample;
    PT(MovieAudioCursor) _stream;
    double               _length;
    int                  _rate;
    int                  _channels;
    int                  _client_count;
    ExpirationQueue::iterator _expire;
  };


  typedef phash_map<std::string, SoundData*> SampleCache;
  SampleCache _sample_cache;

  typedef phash_set<PT(OpenALAudioSound)> SoundsPlaying;
  SoundsPlaying _sounds_playing;

  typedef phash_set<OpenALAudioSound*> AllSounds;
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

  typedef pset<OpenALAudioManager*> Managers;
  static Managers* _managers;

  static ALCdevice* _device;
  static ALCcontext* _context;

  // cache of openal sources, use only for playing sounds
  typedef pset<ALuint > SourceCache;
  static SourceCache* _al_sources;

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
