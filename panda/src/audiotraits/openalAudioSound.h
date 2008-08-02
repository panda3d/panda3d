// Filename: openalAudioSound.h
// Created by:  Ben Buchwald <bb2@alumni.cmu.edu>
//
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



#ifndef __OPENAL_AUDIO_SOUND_H__
#define __OPENAL_AUDIO_SOUND_H__

#include "pandabase.h"

#ifdef HAVE_OPENAL //[

#include "audioSound.h"
#include "movieAudioCursor.h"
#include "trueClock.h"

// OSX uses the OpenAL framework
#ifdef IS_OSX
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
            
  // For best compatability, set the loop_count, start_time,
  // volume, and balance, prior to calling play().  You may
  // set them while they're playing, but it's implementation
  // specific whether you get the results.
  void play();
  void stop();
            
  // loop: false = play once; true = play forever.
  // inits to false.
  void set_loop(bool loop=true);
  bool get_loop() const;
            
  // loop_count: 0 = forever; 1 = play once; n = play n times.
  // inits to 1.
  void set_loop_count(unsigned long loop_count=1);
  unsigned long get_loop_count() const;
            
  // 0 = begining; length() = end.
  // inits to 0.0.
  void set_time(float time=0.0);
  float get_time() const;
            
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  void set_volume(float volume=1.0);
  float get_volume() const;
            
  // -1.0 is hard left
  // 0.0 is centered
  // 1.0 is hard right
  // inits to 0.0.
  void set_balance(float balance_right=0.0);
  float get_balance() const;

  // play_rate is any positive float value.
  // inits to 1.0.
  void set_play_rate(float play_rate=1.0f);
  float get_play_rate() const;

  // inits to manager's state.
  void set_active(bool active=true);
  bool get_active() const;

  // This is the string that throw_event() will throw
  // when the sound finishes playing.  It is not triggered
  // when the sound is stopped with stop().
  void set_finished_event(const string& event);
  const string& get_finished_event() const;

  const string &get_name() const;
  
  // return: playing time in seconds.
  float length() const;

  // Controls the position of this sound's emitter.
  // pos is a pointer to an xyz triplet of the emitter's position.
  // vel is a pointer to an xyz triplet of the emitter's velocity.
  void set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz);
  void get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz);

  void set_3d_min_distance(float dist);
  float get_3d_min_distance() const;

  void set_3d_max_distance(float dist);
  float get_3d_max_distance() const;
            
  void set_3d_drop_off_factor(float factor);
  float get_3d_drop_off_factor() const;

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
  INLINE void require_sound_data();
  INLINE void release_sound_data();
  
private:
  
  void do_stop();
  
  PT(MovieAudio) _movie;
  OpenALAudioManager::SoundData *_sd;

  struct QueuedBuffer {
    ALuint _buffer;
    int    _samples;
    int    _loop_index;
    double _time_offset;
  };

  int    _playing_loops;
  float  _playing_rate;
  
  pdeque<QueuedBuffer> _stream_queued;
  int                  _loops_completed;
  
  ALuint _source;
  PT(OpenALAudioManager) _manager;
  
  float _volume; // 0..1.0
  float _balance; // -1..1
  float _play_rate; // 0..1.0
  
  bool _positional;
  ALfloat _location[3];
  ALfloat _velocity[3];

  float _min_dist;
  float _max_dist;
  float _drop_off_factor;
  
  double _length;
  int    _loop_count;

  int    _desired_mode;

  // The calibrated clock is initialized when the
  // sound starts playing, and is periodically corrected
  // thereafter.  
  double _calibrated_clock_base;
  double _calibrated_clock_scale;
  double _calibrated_clock_decavg;
  
  // The start_time field affects the next call to play.
  double _start_time;

  // The current_time field is updated every frame
  // during the AudioManager update.  Updates need
  // to be atomic, because get_time can be called
  // in the cull thread.
  float  _current_time;
  
  // This is the string that throw_event() will throw
  // when the sound finishes playing.  It is not triggered
  // when the sound is stopped with stop().
  string _finished_event;
  
  Filename _basename;

  // _active is for things like a 'turn off sound effects' in
  // a preferences pannel.
  // _active is not about whether a sound is currently playing.
  // Use status() for info on whether the sound is playing.
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

  ////////////////////////////////////////////////////////////
  //DONE
  ////////////////////////////////////////////////////////////
};

#include "openalAudioSound.I"

#endif //]

#endif /* __OPENAL_AUDIO_SOUND_H__ */





