// Filename: eggScalarTablePointer.cxx
// Created by:  drose (18Jul03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
//     Function: EggScalarTablePointer::get_frame_rate
//       Access: Public, Virtual
//  Description: Returns the stated frame rate of this particular
//               joint, or 0.0 if it doesn't state.
////////////////////////////////////////////////////////////////////
double EggScalarTablePointer::
get_frame_rate() const {
  if (_data == (EggSAnimData *)NULL || !_data->has_fps()) {
    return 0.0;
  } else {
    return _data->get_fps();
  }
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
//     Function: EggScalarTablePointer::extend_to
//       Access: Public, Virtual
//  Description: Extends the table to the indicated number of frames.
////////////////////////////////////////////////////////////////////
void EggScalarTablePointer::
extend_to(int num_frames) {
  nassertv(_data != (EggSAnimData *)NULL);
  int num_rows = _data->get_num_rows();
  double last_value;
  if (num_rows == 0) {
    last_value = 0.0;
  } else {
    last_value = _data->get_value(num_rows - 1);
  }

  while (num_rows < num_frames) {
    _data->add_data(last_value);
    num_rows++;
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

////////////////////////////////////////////////////////////////////
//     Function: EggScalarTablePointer::set_name
//       Access: Public, Virtual
//  Description: Applies the indicated name change to the egg file.
////////////////////////////////////////////////////////////////////
void EggScalarTablePointer::
set_name(const string &name) {
  // Actually, let's not rename the slider table (yet), because we
  // haven't written the code to rename all of the morph targets.

  //  _data->set_name(name);
}
