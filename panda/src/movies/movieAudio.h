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
#include "texture.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : MovieAudio
// Description : A stream that generates a sequence of audio samples.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MovieAudio : public TypedWritableReferenceCount, public Namable {

PUBLISHED:
  MovieAudio(const string &name, double len);
  INLINE int rate() const;
  INLINE int channels() const;
  INLINE int samples_read() const;
  INLINE double approx_len() const;
  INLINE int skip_samples(int n);
  
public:
  virtual int read_samples(int n, PN_int16 *data);
  virtual ~MovieAudio();

private:
  int _rate;
  int _channels;
  int _samples_read;
  double _approx_len;
  
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

#include "movieAudio.I"

#endif
