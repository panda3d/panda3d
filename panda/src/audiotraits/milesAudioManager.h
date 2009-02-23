// Filename: milesAudioManager.h
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef __MILES_AUDIO_MANAGER_H__ //[
#define __MILES_AUDIO_MANAGER_H__

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "audioManager.h"
#include "mss.h"
#include "pset.h"
#include "pmap.h"
#include "pdeque.h"
#include "pvector.h"
#include "thread.h"
#include "pmutex.h"
#include "lightReMutex.h"
#include "conditionVar.h"

class MilesAudioSound;

class EXPCL_MILES_AUDIO MilesAudioManager: public AudioManager {
public:
  // See AudioManager.h for documentation.
  
  MilesAudioManager();
  virtual ~MilesAudioManager();
  
  virtual void shutdown();

  virtual bool is_valid();
  
  virtual PT(AudioSound) get_sound(const string &file_name, bool positional = false, int mode=SM_heuristic);
  virtual PT(AudioSound) get_sound(MovieAudio *sound, bool positional = false, int mode=SM_heuristic);
  virtual void uncache_sound(const string &file_name);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

  virtual void set_volume(float volume);
  virtual float get_volume() const;

  void set_play_rate(float play_rate);
  float get_play_rate() const;
  
  virtual void set_active(bool active);
  virtual bool get_active() const;

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;

  virtual void reduce_sounds_playing_to(unsigned int count);
  virtual void stop_all_sounds();

  virtual void update();

  // Tell the manager that the sound dtor was called.
  void release_sound(MilesAudioSound *audioSound);
  void cleanup();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out) const;

private:
  bool do_is_valid();
  void do_reduce_sounds_playing_to(unsigned int count);
  void do_clear_cache();

  void start_service_stream(HSTREAM stream);
  void stop_service_stream(HSTREAM stream);
  
  void most_recently_used(const string &path);
  void uncache_a_sound();

  void starting_sound(MilesAudioSound *audio);
  void stopping_sound(MilesAudioSound *audio);

  class SoundData;
  PT(SoundData) load(const Filename &file_name);

  void thread_main(volatile bool &keep_running);
  void do_service_streams();

private:
  class StreamThread : public Thread {
  public:
    StreamThread(MilesAudioManager *mgr);
    virtual void thread_main();

    MilesAudioManager *_mgr;
    volatile bool _keep_running;
  };

  // The sound cache:
  class SoundData : public ReferenceCount {
  public:
    SoundData();
    ~SoundData();
    float get_length();
    void set_length(float length);

    Filename _basename;
    S32 _file_type;
    pvector<unsigned char> _raw_data;
    bool _has_length;
    float _length;  // in seconds.
  };
  typedef pmap<string, PT(SoundData) > SoundMap;
  SoundMap _sounds;

  typedef pset<MilesAudioSound *> AudioSet;
  // The offspring of this manager:
  AudioSet _sounds_on_loan;

  typedef pset<MilesAudioSound *> SoundsPlaying;
  // The sounds from this manager that are currently playing:
  SoundsPlaying _sounds_playing;

  // The Least Recently Used mechanism:
  typedef pdeque<const string *> LRU;
  LRU _lru;
  // State:
  float _volume;
  float _play_rate;
  bool _active;
  int _cache_limit;
  bool _cleanup_required;
  unsigned int _concurrent_sound_limit;
  
  bool _is_valid;
  bool _hasMidiSounds;

  // This mutex protects everything above.
  LightReMutex _lock;
  bool _sounds_finished;

  typedef pvector<HSTREAM> Streams;
  PT(StreamThread) _stream_thread;
  Streams _streams;
  Mutex _streams_lock;
  ConditionVar _streams_cvar;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "MilesAudioManager",
                  AudioManager::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class MilesAudioSound;
  friend class MilesAudioSample;
  friend class MilesAudioSequence;
  friend class MilesAudioStream;
};

EXPCL_MILES_AUDIO AudioManager *Create_MilesAudioManager();


#endif //]

#endif //]


