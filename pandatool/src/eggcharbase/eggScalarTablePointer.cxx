/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggScalarTablePointer.cxx
 * @author drose
 * @date 2003-07-18
 */

#include "eggScalarTablePointer.h"

#include "dcast.h"

TypeHandle EggScalarTablePointer::_type_handle;

/**
 *
 */
EggScalarTablePointer::
EggScalarTablePointer(EggObject *object) {
  _data = DCAST(EggSAnimData, object);
}

/**
 * Returns the stated frame rate of this particular joint, or 0.0 if it
 * doesn't state.
 */
double EggScalarTablePointer::
get_frame_rate() const {
  if (_data == nullptr || !_data->has_fps()) {
    return 0.0;
  } else {
    return _data->get_fps();
  }
}

/**
 * Returns the number of frames of animation for this particular slider.
 */
int EggScalarTablePointer::
get_num_frames() const {
  if (_data == nullptr) {
    return 0;
  } else {
    return _data->get_num_rows();
  }
}

/**
 * Extends the table to the indicated number of frames.
 */
void EggScalarTablePointer::
extend_to(int num_frames) {
  nassertv(_data != nullptr);
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

/**
 * Returns the value corresponding to this slider position in the nth frame.
 */
double EggScalarTablePointer::
get_frame(int n) const {
  if (get_num_frames() == 1) {
    // If we have exactly one frame, then we have as many frames as we want;
    // just repeat the first frame.
    n = 0;
  }

  nassertr(n >= 0 && n < get_num_frames(), 0.0);
  return _data->get_value(n);
}

/**
 * Applies the indicated name change to the egg file.
 */
void EggScalarTablePointer::
set_name(const std::string &name) {
  // Actually, let's not rename the slider table (yet), because we haven't
  // written the code to rename all of the morph targets.

  // _data->set_name(name);
}
