// Filename: audioSound.h
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

#ifndef __AUDIOSOUND_H__
#define __AUDIOSOUND_H__

#include "config_audio.h"
#include <typedReferenceCount.h>
#include <typedObject.h>
#include <namable.h>
#include <pointerTo.h>


class AudioManager;

class EXPCL_PANDA AudioSound : public TypedReferenceCount, public Namable {
PUBLISHED:
  virtual ~AudioSound() {}
  
  // For best compatability, set the loop_count, start_time,
  // volume, and balance, prior to calling play().  You may
  // set them while they're playing, but it's implementation
  // specific whether you get the results.
  virtual void play() = 0;
  virtual void stop() = 0;
  
  // loop: 0 = play once; 1 = play forever.
  // inits to false.
  virtual void set_loop(bool loop=false) = 0;
  virtual bool get_loop() const = 0;
  
  // loop_count: 0 = forever; 1 = play once; n = play n times.
  // inits to 1.
  virtual void set_loop_count(unsigned long loop_count=1) = 0;
  virtual unsigned long get_loop_count() const = 0;
  
  // start_time: 0 = begining; length() = end.
  // inits to 0.0.
  virtual void set_time(float start_time=0.0) = 0;
  virtual float get_time() const = 0;
  
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  virtual void set_volume(float volume=1.0) = 0;
  virtual float get_volume() const = 0;
  
  // -1.0 is hard left
  // 0.0 is centered
  // 1.0 is hard right
  // inits to 0.0.
  virtual void set_balance(float balance_right=0.0) = 0;
  virtual float get_balance() const = 0;

  // inits to manager's state.
  virtual void set_active(bool flag=true) = 0;
  virtual bool get_active() const = 0;
  
  // There is no set_name(), this is intentional.
  virtual const string& get_name() const = 0;
  
  // return: playing time in seconds.
  virtual float length() const = 0;

  enum SoundStatus { BAD, READY, PLAYING };
  virtual SoundStatus status() const = 0;

protected:
  AudioSound() {
    // Intentionally blank.
  }

  friend class AudioManager;

public:
  // type stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedReferenceCount::get_class_type());
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

#endif /* __AUDIOSOUND_H__ */
