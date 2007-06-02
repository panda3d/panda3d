// Filename: finiteBoundingVolume.cxx
// Created by:  drose (02Oct99)
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

#include "finiteBoundingVolume.h"
#include "boundingBox.h"

TypeHandle FiniteBoundingVolume::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FiniteBoundingVolume::get_volume
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float FiniteBoundingVolume::
get_volume() const {
  nassertr(!is_infinite(), 0.0f);
  if (is_empty()) {
    return 0.0f;
  }

  mathutil_cat.warning()
    << get_type() << "::get_volume() called\n";

  // We don't know how to compute the volume of this shape correctly;
  // just calculate the volume of its containing box.
  BoundingBox box(get_min(), get_max());
  box.local_object();
  return box.get_volume();
}
