// Filename: eggScalarTablePointer.cxx
// Created by:  drose (18Jul03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eggScalarTablePointer.h"

#include "dcast.h"

TypeHandle EggScalarTablePointer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggScalarTablePointer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggScalarTablePointer::
EggScalarTablePointer(EggObject *object) {
  _data = DCAST(EggSAnimData, object);
}

////////////////////////////////////////////////////////////////////
//     Function: EggScalarTablePointer::get_num_frames
//       Access: Public, Virtual
//  Description: Returns the number of frames of animation for this
//               particular slider.
////////////////////////////////////////////////////////////////////
int EggScalarTablePointer::
get_num_frames() const {
  if (_data == (EggSAnimData *)NULL) {
    return 0;
  } else {
    return _data->get_num_rows();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggScalarTablePointer::get_frame
//       Access: Public, Virtual
//  Description: Returns the value corresponding to this
//               slider position in the nth frame.
////////////////////////////////////////////////////////////////////
double EggScalarTablePointer::
get_frame(int n) const {
  if (get_num_frames() == 1) {
    // If we have exactly one frame, then we have as many frames as we
    // want; just repeat the first frame.
    n = 0;
  }

  nassertr(n >= 0 && n < get_num_frames(), 0.0);
  return _data->get_value(n);
}
