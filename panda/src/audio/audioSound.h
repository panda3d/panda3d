// Filename: audioSound.h
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
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

#ifndef __AUDIOSOUND_H__
#define __AUDIOSOUND_H__

#include "config_audio.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"


class AudioManager;

class EXPCL_PANDA AudioSound : public TypedReferenceCount {
PUBLISHED:
  virtual ~AudioSound();
  
  // For best compatability, set the loop_count,
  // volume, and balance, prior to calling play().  You may
  // set them while they're playing, but it's implementation
  // specific whether you get the results.
  // - Calling play() a second time on the same sound before it is
  //   finished will start the sound again (creating a skipping or
  //   stuttering effect).
  virtual void play() = 0;
  virtual void stop() = 0;
  
  // loop: false = play once; true = play forever.
  // inits to false.
  virtual void set_loop(bool loop=true) = 0;
  virtual bool get_loop() const = 0;
  
  // loop_count: 0 = forever; 1 = play once; n = play n times.
  // inits to 1.
  virtual void set_loop_count(unsigned long loop_count=1) = 0;
  virtual unsigned long get_loop_count() const = 0;
  
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
  
  // play_rate is any positive float value.
  // inits to 1.0.
  virtual void set_play_rate(float play_rate=1.0f) = 0;
  virtual float get_play_rate() const = 0;

  // inits to manager's state.
  virtual void set_active(bool flag=true) = 0;
  virtual bool get_active() const = 0;

  // Set (or clear) the event that will be thrown when the sound
  // finishes playing.  To clear the event, pass an empty string.
  virtual void set_finished_event(const string& event) = 0;
  virtual const string& get_finished_event() const = 0;
  
  // There is no set_name(), this is intentional.
  virtual const string& get_name() const = 0;
  
  // return: playing time in seconds.
  virtual float length() const = 0;

  // Controls the position of this sound's emitter.
  // px, py and pz are the emitter's position.
  // vx, vy and vz are the emitter's velocity in UNITS PER SECOND (default: meters).
  virtual void set_3d_attributes(float px, float py, float pz,
                                 float vx, float vy, float vz);
  virtual void get_3d_attributes(float *px, float *py, float *pz,
                                 float *vx, float *vy, float *vz);


  // Controls the distance (in units) that this sound begins to fall off.
  // Also affects the rate it falls off.
  // Default is 1.0
  // Closer/Faster, <1.0
  // Farther/Slower, >1.0
  virtual void set_3d_min_distance(float dist);
  virtual float get_3d_min_distance() const;

  // Controls the maximum distance (in units) that this sound stops falling off.
  // The sound does not stop at that point, it just doesn't get any quieter.
  // You should rarely need to adjust this.
  // Default is 1000000000.0
  virtual void set_3d_max_distance(float dist);
  virtual float get_3d_max_distance() const;

  enum SoundStatus { BAD, READY, PLAYING };
  virtual SoundStatus status() const = 0;

protected:
  AudioSound();

  friend class AudioManager;

public:
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
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif /* __AUDIOSOUND_H__ */
