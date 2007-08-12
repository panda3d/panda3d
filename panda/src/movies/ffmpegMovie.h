// Filename: ffmpegMovie.h
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

#ifndef FFMPEGMOVIE_H
#define FFMPEGMOVIE_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "movie.h"

class FfmpegVideo;
class FfmpegAudio;

////////////////////////////////////////////////////////////////////
//       Class : FfmpegMovie
// Description : A Movie loader class that uses FFMPEG as its
//               underlying decoder.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegMovie : public Movie {

PUBLISHED:
  FfmpegMovie(const string &filename);

  virtual PT(MovieVideo) get_video(double offset=0.0) const;
  virtual PT(MovieAudio) get_audio(double offset=0.0) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Movie::init_type();
    register_type(_type_handle, "FfmpegMovie",
                  Movie::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  friend class FfmpegVideo;
  friend class FfmpegAudio;
};

#include "ffmpegMovie.I"

#endif
