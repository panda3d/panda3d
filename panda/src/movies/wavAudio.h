// Filename: wavAudio.h
// Created by: rdb (23Aug13)
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

#ifndef WAVAUDIO_H
#define WAVAUDIO_H

#include "pandabase.h"
#include "movieAudio.h"

class WavAudioCursor;

////////////////////////////////////////////////////////////////////
//       Class : WavAudio
// Description : A native PCM .wav loader.  Supported formats
//               are linear PCM, IEEE float, A-law and mu-law.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES WavAudio : public MovieAudio {
PUBLISHED:
  WavAudio(const Filename &name);
  virtual ~WavAudio();
  virtual PT(MovieAudioCursor) open();

  static PT(MovieAudio) make(const Filename &name);

private:
  friend class WavAudioCursor;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "WavAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wavAudio.I"

#endif
