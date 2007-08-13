// Filename: ffmpegAudio.h
// Created by: jyelon (01Aug2007)
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

#ifndef FFMPEGAUDIO_H
#define FFMPEGAUDIO_H
#ifdef HAVE_FFMPEG

#include "pandabase.h"
#include "namable.h"
#include "texture.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : FfmpegAudio
// Description : A stream that generates a sequence of audio samples.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegAudio : public MovieAudio {

PUBLISHED:
  FfmpegAudio(const Filename &name);
  virtual ~FfmpegAudio();
  virtual PT(MovieAudio) make_copy() const;

public:
  virtual void read_samples(int n, PN_int16 *data);
  
protected:
  Filename _filename;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "FfmpegAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "ffmpegAudio.I"

#endif // HAVE_FFMPEG
#endif // FFMPEG_AUDIO.H
