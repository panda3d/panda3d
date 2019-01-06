/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file milesAudioManager.h
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#ifndef __MILES_AUDIO_MANAGER_H__ //[
#define __MILES_AUDIO_MANAGER_H__

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "audioManager.h"
#include "pset.h"
#include "pmap.h"
#include "pdeque.h"
#include "pvector.h"
#include "thread.h"
#include "pmutex.h"
#include "lightReMutex.h"
#include "conditionVar.h"
#include "vector_uchar.h"

#include <mss.h>

class MilesAudioSound;

class EXPCL_MILES_AUDIO MilesAudioManager: public AudioManager {
public:
  // See AudioManager.h for documentation.

  MilesAudioManager();
  virtual ~MilesAudioManager();

  virtual void shutdown();

  virtual bool is_valid();

  virtual PT(AudioSound) get_sound(const Filename &file_name, bool positional = false, int mode=SM_heuristic);
  virtual PT(AudioSound) get_sound(MovieAudio *sound, bool positional = false, int mode=SM_heuristic);
  virtual void uncache_sound(const Filename &file_name);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

  virtual void set_volume(PN_stdfloat volume);
  virtual PN_stdfloat get_volume() const;

  void set_play_rate(PN_stdfloat play_rate);
  PN_stdfloat get_play_rate() const;

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

  // 3D spatialized sound support.  Spatialized sound was originally added for
  // FMOD, so there are parts of the interface in the Miles implementation
  // that are a little more awkward than they would be otherwise.
  virtual void audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat xy, PN_stdfloat xz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz);
  virtual void audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz);
  virtual void audio_3d_set_distance_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_distance_factor() const;
  virtual void audio_3d_set_doppler_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_doppler_factor() const;
  virtual void audio_3d_set_drop_off_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_drop_off_factor() const;
  virtual void set_speaker_configuration(LVecBase3 *speaker1, LVecBase3 *speaker2=nullptr, LVecBase3 *speaker3=nullptr, LVecBase3 *speaker4=nullptr, LVecBase3 *speaker5=nullptr, LVecBase3 *speaker6=nullptr, LVecBase3 *speaker7=nullptr, LVecBase3 *speaker8=nullptr, LVecBase3 *speaker9=nullptr);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out) const;

private:
  bool do_is_valid();
  void do_reduce_sounds_playing_to(unsigned int count);
  void do_clear_cache();

  void start_service_stream(HSTREAM stream);
  void stop_service_stream(HSTREAM stream);

  void most_recently_used(const std::string &path);
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
    PN_stdfloat get_length();
    void set_length(PN_stdfloat length);

    Filename _basename;
    S32 _file_type;
    vector_uchar _raw_data;
    bool _has_length;
    PN_stdfloat _length;  // in seconds.
  };
  typedef pmap<std::string, PT(SoundData) > SoundMap;
  SoundMap _sounds;

  typedef pset<MilesAudioSound *> AudioSet;
  // The offspring of this manager:
  AudioSet _sounds_on_loan;

  typedef pset<MilesAudioSound *> SoundsPlaying;
  // The sounds from this manager that are currently playing:
  SoundsPlaying _sounds_playing;

  // The Least Recently Used mechanism:
  typedef pdeque<const std::string *> LRU;
  LRU _lru;
  // State:
  PN_stdfloat _volume;
  PN_stdfloat _play_rate;
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
