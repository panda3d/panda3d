/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inkblotVideo.cxx
 * @author jyelon
 * @date 2007-07-02
 */

#include "inkblotVideo.h"
#include "inkblotVideoCursor.h"

TypeHandle InkblotVideo::_type_handle;

/**
 * xxx
 */
InkblotVideo::
InkblotVideo(int x, int y, int fps) :
  _specified_x(x),
  _specified_y(y),
  _specified_fps(fps)
{
}

/**
 *
 */
InkblotVideo::
~InkblotVideo() {
}

/**
 * Open this video, returning a MovieVideoCursor.
 */
PT(MovieVideoCursor) InkblotVideo::
open() {
  return new InkblotVideoCursor(this);
}
