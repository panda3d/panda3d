// Filename: movieAudio.h
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

#ifndef MOVIEAUDIO_H
#define MOVIEAUDIO_H

#include "pandabase.h"
#include "namable.h"
#include "pointerTo.h"
#include "typedWritableReferenceCount.h"
class MovieAudioCursor;

////////////////////////////////////////////////////////////////////
//       Class : MovieAudio
// Description : A MovieAudio is actually any source that provides
//               a sequence of audio samples.  That could include an
//               AVI file, a microphone, or an internet TV station.
//
//               The difference between a MovieAudio and a
//               MovieAudioCursor is like the difference between a
//               filename and a file handle.  The MovieAudio just
//               indicates a particular movie.  The MovieAudioCursor
//               is what allows access.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES MovieAudio : public TypedWritableReferenceCount, public Namable {

 PUBLISHED:
  MovieAudio(const string &name = "Blank Audio");
  virtual ~MovieAudio();
  virtual PT(MovieAudioCursor) open();
  static PT(MovieAudio) get(const Filename &name);
  INLINE const Filename &get_filename() const;

 protected:
  Filename _filename;

 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieAudio",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

/* okcircular */
#include "movieAudioCursor.h"

#include "movieAudio.I"

#endif
