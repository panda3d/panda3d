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
protected:
  MovieVideo();

PUBLISHED:
  virtual ~MovieVideo();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsOutput::init_type();
    register_type(_type_handle, "MovieVideo",
                  MovieVideo::get_class_type());
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
