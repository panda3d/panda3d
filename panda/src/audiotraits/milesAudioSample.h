// Filename: milesAudioSample.h
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

#ifndef MILESAUDIOSAMPLE_H
#define MILESAUDIOSAMPLE_H

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "milesAudioSound.h"
#include "milesAudioManager.h"
#include "mss.h"

////////////////////////////////////////////////////////////////////
//       Class : MilesAudioSample
// Description : A sound file, such as a WAV or MP3 file, that is
//               preloaded into memory and played from memory.
////////////////////////////////////////////////////////////////////
class EXPCL_MILES_AUDIO MilesAudioSample : public MilesAudioSound {
private:
  MilesAudioSample(MilesAudioManager *manager, 
                   MilesAudioManager::SoundData *sd,
                   const string &file_name);

public:
  virtual ~MilesAudioSample();
  
  virtual void play();
  virtual void stop();
  
  virtual void set_time(float time=0.0f);
  virtual float get_time() const;
  
  virtual void set_volume(float volume=1.0f);
  virtual void set_balance(float balance_right=0.0f);
  virtual void set_play_rate(float play_rate=1.0f);
  
  virtual float length() const;

  virtual AudioSound::SoundStatus status() const;

  virtual void cleanup();

private:
  void internal_stop();

  PT(MilesAudioManager::SoundData) _sd;
  HSAMPLE _sample;
  size_t _sample_index;
  S32 _original_playback_rate;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MilesAudioSound::init_type();
    register_type(_type_handle, "MilesAudioSample",
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

#include "milesAudioSample.I"

#endif //]

#endif  /* MILESAUDIOSAMPLE_H */
