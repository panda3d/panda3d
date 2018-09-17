/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openalAudioSound.h
 * @author Ben Buchwald <bb2@alumni.cmu.edu>
 */

#ifndef __OPENAL_AUDIO_SOUND_H__
#define __OPENAL_AUDIO_SOUND_H__

#include "pandabase.h"

#include "audioSound.h"
#include "movieAudioCursor.h"
#include "trueClock.h"
#include "openalAudioManager.h"

// OSX uses the OpenAL framework
#ifdef HAVE_OPENAL_FRAMEWORK
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

class EXPCL_OPENAL_AUDIO OpenALAudioSound : public AudioSound {
  friend class OpenALAudioManager;

public:

  ~OpenALAudioSound();

  // For best compatibility, set the loop_count, start_time, volume, and
  // balance, prior to calling play().  You may set them while they're
  // playing, but it's implementation specific whether you get the results.
  void play();
  void stop();

  // loop: false = play once; true = play forever.  inits to false.
  void set_loop(bool loop=true);
  bool get_loop() const;

  // loop_count: 0 = forever; 1 = play once; n = play n times.  inits to 1.
  void set_loop_count(unsigned long loop_count=1);
  unsigned long get_loop_count() const;

  // 0 = beginning; length() = end.  inits to 0.0.
  void set_time(PN_stdfloat time=0.0);
  PN_stdfloat get_time() const;

  // 0 = minimum; 1.0 = maximum.  inits to 1.0.
  void set_volume(PN_stdfloat volume=1.0);
  PN_stdfloat get_volume() const;

  // -1.0 is hard left 0.0 is centered 1.0 is hard right inits to 0.0.
  void set_balance(PN_stdfloat balance_right=0.0);
  PN_stdfloat get_balance() const;

  // play_rate is any positive float value.  inits to 1.0.
  void set_play_rate(PN_stdfloat play_rate=1.0f);
  PN_stdfloat get_play_rate() const;

  // inits to manager's state.
  void set_active(bool active=true);
  bool get_active() const;

  // This is the string that throw_event() will throw when the sound finishes
  // playing.  It is not triggered when the sound is stopped with stop().
  void set_finished_event(const std::string& event);
  const std::string& get_finished_event() const;

  const std::string &get_name() const;

  // return: playing time in seconds.
  PN_stdfloat length() const;

  // Controls the position of this sound's emitter.  pos is a pointer to an
  // xyz triplet of the emitter's position.  vel is a pointer to an xyz
  // triplet of the emitter's velocity.
  void set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz);
  void get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz);

  void set_3d_min_distance(PN_stdfloat dist);
  PN_stdfloat get_3d_min_distance() const;

  void set_3d_max_distance(PN_stdfloat dist);
  PN_stdfloat get_3d_max_distance() const;

  void set_3d_drop_off_factor(PN_stdfloat factor);
  PN_stdfloat get_3d_drop_off_factor() const;

  AudioSound::SoundStatus status() const;

  void finished();

private:
  OpenALAudioSound(OpenALAudioManager* manager,
                   MovieAudio *movie,
                   bool positional,
                   int mode);
  INLINE void   set_calibrated_clock(double rtc, double t, double playrate);
  INLINE double get_calibrated_clock(double rtc) const;
  void          correct_calibrated_clock(double rtc, double t);
  void          cache_time(double rtc);
  void cleanup();
  void restart_stalled_audio();
  void delete_queued_buffers();
  ALuint make_buffer(int samples, int channels, int rate, unsigned char *data);
  void queue_buffer(ALuint buffer, int samples, int loop_index, double time_offset);
  int  read_stream_data(int bytelen, unsigned char *data);
  void pull_used_buffers();
  void push_fresh_buffers();
  INLINE bool require_sound_data();
  INLINE void release_sound_data(bool force);

  INLINE bool is_valid() const;
  INLINE bool is_playing() const;
  INLINE bool has_sound_data() const;

private:

  PT(MovieAudio) _movie;
  OpenALAudioManager::SoundData *_sd;

  struct QueuedBuffer {
    ALuint _buffer;
    int    _samples;
    int    _loop_index;
    double _time_offset;
  };

  int    _playing_loops;
  PN_stdfloat  _playing_rate;

  pdeque<QueuedBuffer> _stream_queued;
  int                  _loops_completed;

  ALuint _source;
  PT(OpenALAudioManager) _manager;

  PN_stdfloat _volume; // 0..1.0
  PN_stdfloat _balance; // -1..1
  PN_stdfloat _play_rate; // 0..1.0

  bool _positional;
  ALfloat _location[3];
  ALfloat _velocity[3];

  PN_stdfloat _min_dist;
  PN_stdfloat _max_dist;
  PN_stdfloat _drop_off_factor;

  double _length;
  int    _loop_count;

  int    _desired_mode;

  // The calibrated clock is initialized when the sound starts playing, and is
  // periodically corrected thereafter.
  double _calibrated_clock_base;
  double _calibrated_clock_scale;
  double _calibrated_clock_decavg;

  // The start_time field affects the next call to play.
  double _start_time;

  // The current_time field is updated every frame during the AudioManager
  // update.  Updates need to be atomic, because get_time can be called in the
  // cull thread.
  PN_stdfloat  _current_time;

  // This is the string that throw_event() will throw when the sound finishes
  // playing.  It is not triggered when the sound is stopped with stop().
  std::string _finished_event;

  Filename _basename;

  // _active is for things like a 'turn off sound effects' in a preferences
  // pannel.  _active is not about whether a sound is currently playing.  Use
  // status() for info on whether the sound is playing.
  bool _active;
  bool _paused;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioSound::init_type();
    register_type(_type_handle, "OpenALAudioSound", AudioSound::get_class_type());
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

#include "openalAudioSound.I"

#endif /* __OPENAL_AUDIO_SOUND_H__ */
