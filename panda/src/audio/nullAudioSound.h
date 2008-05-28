// Filename: nullAudioSound.h
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

#ifndef __NULL_AUDIO_SOUND_H__
#define __NULL_AUDIO_SOUND_H__

#include "audioSound.h"


// This class intentionally does next to nothing.
// It's used as a placeholder when you don't want a sound
// system.
class EXPCL_PANDA_AUDIO NullAudioSound : public AudioSound {
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
  
  void set_play_rate(float);
  float get_play_rate() const;

  void set_active(bool);
  bool get_active() const;

  void set_finished_event(const string& event);
  const string& get_finished_event() const;
  
  const string& get_name() const;
  
  float length() const;

  void set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz);
  void get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz);
  void set_3d_min_distance(float dist);
  float get_3d_min_distance() const;
  void set_3d_max_distance(float dist);
  float get_3d_max_distance() const;
  
  AudioSound::SoundStatus status() const;

// why protect the constructor?!?
//protected:
  NullAudioSound();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioSound::init_type();
    register_type(_type_handle, "NullAudioSound",
                  AudioSound::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class NullAudioManager;
};

#endif /* __NULL_AUDIO_SOUND_H__ */
