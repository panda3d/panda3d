// Filename: milesAudioSequence.h
// Created by:  drose (31Jul07)
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

#ifndef MILESAUDIOSEQUENCE_H
#define MILESAUDIOSEQUENCE_H

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "milesAudioSound.h"
#include "milesAudioManager.h"
#include "mss.h"

////////////////////////////////////////////////////////////////////
//       Class : MilesAudioSequence
// Description : A MIDI file, preloaded and played from a memory
//               buffer.  MIDI files cannot be streamed.
////////////////////////////////////////////////////////////////////
class EXPCL_MILES_AUDIO MilesAudioSequence : public MilesAudioSound {
private:
  MilesAudioSequence(MilesAudioManager *manager, 
                     MilesAudioManager::SoundData *sd,
                     const string &file_name);

public:
  virtual ~MilesAudioSequence();
  
  virtual void play();
  virtual void stop();
  
  virtual PN_stdfloat get_time() const;
  
  virtual void set_volume(PN_stdfloat volume=1.0f);
  virtual void set_balance(PN_stdfloat balance_right=0.0f);
  virtual void set_play_rate(PN_stdfloat play_rate=1.0f);
  
  virtual PN_stdfloat length() const;

  virtual AudioSound::SoundStatus status() const;

  virtual void cleanup();

private:
  void internal_stop();
  static void AILCALLBACK finish_callback(HSEQUENCE sequence);
  void do_set_time(PN_stdfloat time);
  void determine_length();

  PT(MilesAudioManager::SoundData) _sd;
  HSEQUENCE _sequence;
  size_t _sequence_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MilesAudioSound::init_type();
    register_type(_type_handle, "MilesAudioSequence",
                  MilesAudioSound::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GlobalMilesManager;
  friend class MilesAudioManager;
};

#include "milesAudioSequence.I"

#endif //]

#endif  /* MILESAUDIOSEQUENCE_H */
