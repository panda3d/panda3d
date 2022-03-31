/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nullAudioSound.h
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#ifndef NULLAUDIOSOUND_H
#define NULLAUDIOSOUND_H

#include "audioSound.h"


// This class intentionally does next to nothing.  It's used as a placeholder
// when you don't want a sound system.
class EXPCL_PANDA_AUDIO NullAudioSound : public AudioSound {
  // All of these methods are stubbed out to some degree.  If you're looking
  // for a starting place for a new AudioManager, please consider looking at
  // the openalAudioManager.

public:
  ~NullAudioSound();

  void play();
  void stop();

  void set_loop(bool);
  bool get_loop() const;

  void set_loop_count(unsigned long);
  unsigned long get_loop_count() const;

  void set_time(PN_stdfloat);
  PN_stdfloat get_time() const;

  void set_volume(PN_stdfloat);
  PN_stdfloat get_volume() const;

  void set_balance(PN_stdfloat);
  PN_stdfloat get_balance() const;

  void set_play_rate(PN_stdfloat);
  PN_stdfloat get_play_rate() const;

  void set_active(bool);
  bool get_active() const;

  void set_finished_event(const std::string& event);
  const std::string& get_finished_event() const;

  const std::string& get_name() const;

  PN_stdfloat length() const;

  void set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz);
  void get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz);
  void set_3d_min_distance(PN_stdfloat dist);
  PN_stdfloat get_3d_min_distance() const;
  void set_3d_max_distance(PN_stdfloat dist);
  PN_stdfloat get_3d_max_distance() const;

  AudioSound::SoundStatus status() const;

// why protect the constructor?!? protected:
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

#endif /* NULLAUDIOSOUND_H */
