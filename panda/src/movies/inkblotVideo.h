// Filename: inkblotVideo.h
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

#ifndef INKBLOTVIDEO_H
#define INKBLOTVIDEO_H

#include "movieVideo.h"

class InkblotVideoCursor;

////////////////////////////////////////////////////////////////////
//       Class : InkblotVideo
// Description : A cellular automaton that generates an amusing
//               pattern of swirling colors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES InkblotVideo : public MovieVideo {

 PUBLISHED:
  InkblotVideo(int x, int y, int fps);
  virtual ~InkblotVideo();
  virtual PT(MovieVideoCursor) open();

 private:
  int _specified_x;
  int _specified_y;
  int _specified_fps;
  friend class InkblotVideoCursor;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideo::init_type();
    register_type(_type_handle, "InkblotVideo",
                  MovieVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "inkblotVideo.I"

#endif
