// Filename: nullAudioSound.h
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

#ifndef __NULL_AUDIO_SOUND_H__
#define __NULL_AUDIO_SOUND_H__

#include "audioSound.h"


// This class intentionally does next to nothing.
// It's used as a placeholder when you don't want a sound
// system.
class EXPCL_PANDA NullAudioSound : public AudioSound {
public:
  ~NullAudioSound();
  
  // For best compatability, set the loop_count, start_time,
  // volume, and balance, prior to calling play().  You may
  // set them while they're playing, but it's implementation
  // specific whether you get the results.
  void play();
  void stop();
  
  // loop: 0 = play once; 1 = play forever.
  // inits to false.
  void set_loop(bool);
  bool get_loop() const;
  
  // loop_count: 0 = forever; 1 = play once; n = play n times.
  // inits to 1.
  void set_loop_count(unsigned long);
  unsigned long get_loop_count() const;
  
  // start_time: 0 = begining; length() = end.
  // inits to 0.0.
  void set_time(float);
  float get_time() const;
  
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  void set_volume(float);
  float get_volume() const;
  
  // -1.0 is hard left
  // 0.0 is centered
  // 1.0 is hard right
  // inits to 0.0.
  void set_balance(float);
  float get_balance() const;

  // inits to manager's state.
  void set_active(bool);
  bool get_active() const;
  
  // There is no set_name(), this is intentional.
  const string& get_name() const;
  
  // return: playing time in seconds.
  float length() const;

  AudioSound::SoundStatus status() const;

protected:
  NullAudioSound();

  friend class NullAudioManager;
};

#endif /* __NULL_AUDIO_SOUND_H__ */
