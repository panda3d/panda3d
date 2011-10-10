// Filename: milesAudioStream.h
// Created by:  drose (26Jul07)
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

#ifndef MILESAUDIOSTREAM_H
#define MILESAUDIOSTREAM_H

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "milesAudioSound.h"
#include "milesAudioManager.h"
#include "mss.h"

////////////////////////////////////////////////////////////////////
//       Class : MilesAudioStream
// Description : This represents a sound file played by the Miles
//               Sound System, similar to MilesAudioSample, except
//               that it is streamed from disk instead of preloaded
//               into memory.
////////////////////////////////////////////////////////////////////
class EXPCL_MILES_AUDIO MilesAudioStream : public MilesAudioSound {
private:
  MilesAudioStream(MilesAudioManager *manager, const string &file_name,
                   const Filename &path);

public:
  virtual ~MilesAudioStream();
  
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
  static void AILCALLBACK finish_callback(HSTREAM stream);
  void do_set_time(PN_stdfloat time);

  Filename _path;
  HSTREAM _stream;
  S32 _original_playback_rate;
  mutable PN_stdfloat _length;
  mutable bool _got_length;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MilesAudioSound::init_type();
    register_type(_type_handle, "MilesAudioStream",
                  MilesAudioSound::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class MilesAudioManager;
};

#include "milesAudioStream.I"

#endif //]

#endif
