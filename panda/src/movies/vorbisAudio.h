// Filename: vorbisAudio.h
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

#ifndef VORBISAUDIO_H
#define VORBISAUDIO_H

#include "pandabase.h"
#include "movieAudio.h"

#ifdef HAVE_VORBIS

class VorbisAudioCursor;

////////////////////////////////////////////////////////////////////
//       Class : VorbisAudio
// Description : Interfaces with the libvorbisfile library to
//               implement decoding of Ogg Vorbis audio files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES VorbisAudio : public MovieAudio {
PUBLISHED:
  VorbisAudio(const Filename &name);
  virtual ~VorbisAudio();
  virtual PT(MovieAudioCursor) open();

  static PT(MovieAudio) make(const Filename &name);

private:
  friend class VorbisAudioCursor;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "VorbisAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vorbisAudio.I"

#endif // HAVE_VORBIS

#endif // VORBISAUDIO_H
