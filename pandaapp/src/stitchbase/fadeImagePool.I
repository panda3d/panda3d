// Filename: fadeImagePool.I
// Created by:  drose (30Jul01)
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


////////////////////////////////////////////////////////////////////
//     Function: FadeImagePool::get_image
//       Access: Public
//  Description: Returns a pointer to the fade image of the indicated
//               filename, scaled to the indicated size, or NULL if
//               the image cannot be loaded.
////////////////////////////////////////////////////////////////////
INLINE const PNMImage *FadeImagePool::
get_image(const Filename &filename, int x_size, int y_size) {
  return get_ptr()->ns_get_image(filename, x_size, y_size);
}
