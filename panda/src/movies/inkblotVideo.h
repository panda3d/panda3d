/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inkblotVideo.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef INKBLOTVIDEO_H
#define INKBLOTVIDEO_H

#include "movieVideo.h"

class InkblotVideoCursor;

/**
 * A cellular automaton that generates an amusing pattern of swirling colors.
 */
class EXPCL_PANDA_MOVIES InkblotVideo : public MovieVideo {
PUBLISHED:
  explicit InkblotVideo(int x, int y, int fps);
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
