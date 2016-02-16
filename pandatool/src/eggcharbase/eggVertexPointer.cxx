/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexPointer.cxx
 * @author drose
 * @date 2001-02-26
 */

#include "eggVertexPointer.h"


TypeHandle EggVertexPointer::_type_handle;

/**

 */
EggVertexPointer::
EggVertexPointer(EggObject *egg_object) {
}

/**
 * Returns the number of frames of animation for this particular slider.
 */
int EggVertexPointer::
get_num_frames() const {
  return 0;
}

/**
 * Returns the value corresponding to this slider position in the nth frame.
 */
double EggVertexPointer::
get_frame(int n) const {
  nassertr(false, 0.0);
  return 0.0;
}

/**
 * Returns true if there are any vertices referenced by the node this points to,
 * false otherwise.  For certain kinds of back pointers (e.g.  table animation
 * entries), this is always false.
 */
bool EggVertexPointer::
has_vertices() const {
  return true;
}
