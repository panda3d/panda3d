// Filename: movieVideo.h
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

#ifndef MOVIEVIDEO_H
#define MOVIEVIDEO_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : MovieVideo
// Description : A stream that generates a series of images.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MovieVideo : public TypedWritableReferenceCount, public Namable {

 PUBLISHED:
  MovieVideo(const string &name, double len);
  INLINE int size_x() const;
  INLINE int size_y() const;
  INLINE double frame_start() const;
  INLINE double frame_end() const;
  INLINE double approx_len() const;
  virtual void load_image(Texture *t);
  virtual void next_frame();
  
 public:
  virtual ~MovieVideo();
  
 private:
  int _size_x;
  int _size_y;
  double _frame_start;
  double _frame_end;
  double _approx_len;
  PTA_uchar _ram_image;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieVideo",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movieVideo.I"

#endif
