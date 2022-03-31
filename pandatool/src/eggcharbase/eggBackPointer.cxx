/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBackPointer.cxx
 * @author drose
 * @date 2001-02-26
 */

#include "eggBackPointer.h"

#include "pnotify.h"


TypeHandle EggBackPointer::_type_handle;

/**
 *
 */
EggBackPointer::
EggBackPointer() {
}

/**
 * Returns the stated frame rate of this particular joint, or 0.0 if it
 * doesn't state.
 */
double EggBackPointer::
get_frame_rate() const {
  return 0.0;
}

/**
 * Extends the table to the indicated number of frames.
 */
void EggBackPointer::
extend_to(int num_frames) {
  // Whoops, can't extend this kind of table!
  nassert_raise("can't extend this kind of table");
}

/**
 * Returns true if there are any vertices referenced by the node this points
 * to, false otherwise.  For certain kinds of back pointers (e.g.  table
 * animation entries), this is always false.
 */
bool EggBackPointer::
has_vertices() const {
  return false;
}

/**
 * Applies the indicated name change to the egg file.
 */
void EggBackPointer::
set_name(const std::string &name) {
}
