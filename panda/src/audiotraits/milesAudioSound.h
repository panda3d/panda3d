// Filename: milesAudioSound.h
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

#ifndef __MILES_AUDIO_SOUND_H__
#define __MILES_AUDIO_SOUND_H__

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "audioSound.h"
#include "milesAudioManager.h"
#include "mss.h"
  
class AutoAilLock {
  // This will lock and unlock the Miles AIL timer based
  // on the current code block.  (Auto in the class name
  // is referring to "auto variable").
public:
  AutoAilLock() {
    AIL_lock();
  }
  ~AutoAilLock() {
    AIL_unlock();
  }
};

class EXPCL_MILES_AUDIO MilesAudioSound : public AudioSound {
public:
  ~MilesAudioSound();
  
  // For best compatability, set the loop_count,
  // volume, and balance, prior to calling play().  You may
  // set them while they're playing, but it's implementation
  // specific whether you get the results.
  // - Calling play() a second time on the same sound before it is
  //   finished will start the sound again (creating a skipping or
  //   stuttering effect).
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
  
  // Control time position within the sound.
  // This is similar (in concept) to the seek position within
  // a file.
  // time in seconds: 0 = beginning; length() = end.
  // inits to 0.0.
  // - Unlike the other get_* and set_* calls for a sound, the
  //   current time position will change while the sound is playing.
  //   To play the same sound from a time offset a second time,
  //   explicitly set the time position again.  When looping, the
  //   second and later loops will start from the beginning of the
  //   sound.
  // - If a sound is playing, calling get_time() repeatedly will
  //   return different results over time.  e.g.:
  //   float percent_complete = s.get_time() / s.length();
  void set_time(float time=0.0f);
  float get_time() const;
  
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  void set_volume(float volume=1.0f);
  float get_volume() const;
  
  // -1.0 is hard left
  // 0.0 is centered
  // 1.0 is hard right
  // inits to 0.0.
  void set_balance(float balance_right=0.0f);
  float get_balance() const;

  // inits to manager's state.
  void set_active(bool active=true);
  bool get_active() const;

  // This is the string that throw_event() will throw
  // when the sound finishes playing.  It is not triggered
  // when the sound is stopped with stop().
  void set_finished_event(const string& event);
  const string& get_finished_event() const;
  
  const string& get_name() const;
  
  // return: playing time in seconds.
  float length() const;

  AudioSound::SoundStatus status() const;

  void finished();

private:
  HAUDIO _audio;
  PT(MilesAudioManager) _manager;
  float _volume; // 0..1.0
  float _balance; // -1..1
  mutable float _length; // in seconds.
  unsigned long _loop_count;
  
  // This is the string that throw_event() will throw
  // when the sound finishes playing.  It is not triggered
  // when the sound is stopped with stop().
  string _finished_event;
  
  string _file_name;
  
  // _active is for things like a 'turn off sound effects' in
  // a preferences pannel.
  // _active is not about whether a sound is currently playing.
  // Use status() for info on whether the sound is playing.
  bool _active;
  
  // _paused is not like the Pause button on a cd/dvd player.
  // It is used as a flag to say that the sound was looping when
  // itwas set inactive.
  bool _paused;

  MilesAudioSound(MilesAudioManager* manager, 
      HAUDIO audio, string file_name, float length=0.0f);

  friend class MilesAudioManager;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioSound::init_type();
    register_type(_type_handle, "MilesAudioSound",
                  AudioSound::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "milesAudioSound.I"

#endif //]

#endif /* __MILES_AUDIO_SOUND_H__ */
