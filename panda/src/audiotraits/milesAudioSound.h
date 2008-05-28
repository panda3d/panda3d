// Filename: milesAudioSound.h
// Created by:  drose (30Jul07)
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

#ifndef MILESAUDIOSOUND_H
#define MILESAUDIOSOUND_H

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "audioSound.h"
#include "milesAudioManager.h"
#include "mss.h"

////////////////////////////////////////////////////////////////////
//       Class : MilesAudioSound
// Description : The base class for both MilesAudioStream and
//               MilesAudioSample.
////////////////////////////////////////////////////////////////////
class EXPCL_MILES_AUDIO MilesAudioSound : public AudioSound {
protected:
  MilesAudioSound(MilesAudioManager *manager, const string &file_name);

public:  
  virtual void set_loop(bool loop=true);
  virtual bool get_loop() const;
  
  virtual void set_loop_count(unsigned long loop_count=1);
  virtual unsigned long get_loop_count() const;
  
  virtual float get_volume() const;
  virtual float get_balance() const;
  virtual float get_play_rate() const;

  virtual void set_time(float start_time=0.0);

  virtual void set_active(bool active=true);
  virtual bool get_active() const;

  virtual void set_finished_event(const string &event);
  virtual const string &get_finished_event() const;
  
  virtual const string &get_name() const;

  virtual void cleanup();

protected:
  PT(MilesAudioManager) _manager;
  string _file_name;

  float _volume; // 0..1.0
  float _balance; // -1..1
  float _play_rate; // 0..1.0
  unsigned long _loop_count;
  
  // _active is for things like a 'turn off sound effects' in
  // a preferences pannel.
  // _active is not about whether a sound is currently playing.
  // Use status() for info on whether the sound is playing.
  bool _active;
  
  // _paused is not like the Pause button on a cd/dvd player.
  // It is used as a flag to say that the sound was looping when
  // it was set inactive.
  bool _paused;
  
  // This is the string that throw_event() will throw when the sound
  // finishes playing.  It is not triggered when the sound is stopped
  // with stop().  Note: no longer implemented.
  string _finished_event;

  // This is set whenever we call set_time().  Calling play() will
  // respect this if it is set, and then reset it.
  float _start_time;
  bool _got_start_time;

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

  friend class MilesAudioManager;
};

#include "milesAudioSound.I"

#endif //]

#endif
