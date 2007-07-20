// Filename: movie.h
// Created by: jyelon (02Jul07)
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

#ifndef MOVIE_H
#define MOVIE_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "movieVideo.h"
#include "movieAudio.h"

////////////////////////////////////////////////////////////////////
//       Class : Movie
// Description : A "movie" is actually any source that provides
//               an audio and a video stream.  So that could include
//               an AVI file, or an internet TV station.  It could
//               also be an MP3 file paired with a dummy video stream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES Movie : public TypedWritableReferenceCount, public Namable{

PUBLISHED:
  Movie(const string &name, double len);
  INLINE bool ignores_offset() const;
  INLINE bool dummy_video() const;
  INLINE bool dummy_audio() const;
  virtual PT(MovieVideo) get_video(double offset=0.0);
  virtual PT(MovieAudio) get_audio(double offset=0.0);
  
public:
  virtual ~Movie();
  
private:
  bool _ignores_offset;
  bool _dummy_video;
  bool _dummy_audio;
  double _dummy_len;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Movie",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movie.I"

#endif
