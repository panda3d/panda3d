// Filename: nullAudioSound.h
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

#ifndef __NULL_AUDIO_SOUND_H__
#define __NULL_AUDIO_SOUND_H__

#include "audioSound.h"


// This class intentionally does next to nothing.
// It's used as a placeholder when you don't want a sound
// system.
class EXPCL_PANDA NullAudioSound : public AudioSound {
  // All of these methods are stubbed out to some degree.
  // If you're looking for a starting place for a new AudioManager,
  // please consider looking at the milesAudioManager.

public:
  ~NullAudioSound();
  
  void play();
  void stop();
  
  void set_loop(bool);
  bool get_loop() const;
  
  void set_loop_count(unsigned long);
  unsigned long get_loop_count() const;
  
  void set_time(float);
  float get_time() const;
  
  void set_volume(float);
  float get_volume() const;
  
  void set_balance(float);
  float get_balance() const;

  void set_active(bool);
  bool get_active() const;

  void set_finished_event(const string& event);
  const string& get_finished_event() const;
  
  const string& get_name() const;
  
  float length() const;

  void set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz);
  void get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz);
  
  AudioSound::SoundStatus status() const;

// why protect the constructor?!?
//protected:
  NullAudioSound();

  friend class NullAudioManager;
};

#endif /* __NULL_AUDIO_SOUND_H__ */
