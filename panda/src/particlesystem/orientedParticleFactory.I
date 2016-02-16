/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orientedParticleFactory.I
 * @author charles
 * @date 2000-08-16
 */

////////////////////////////////////////////////////////////////////
//     Function: set_initial_orientation
//       Access: Public
////////////////////////////////////////////////////////////////////
void OrientedParticleFactory::
set_initial_orientation(const LOrientation &o) {
  _initial_orientation = o;
}

////////////////////////////////////////////////////////////////////
//     Function: set_final_orientation
//       Access: Public
////////////////////////////////////////////////////////////////////
void OrientedParticleFactory::
set_final_orientation(const LOrientation &o) {
  _final_orientation = o;
}

////////////////////////////////////////////////////////////////////
//     Function: get_initial_orientation
//       Access: Public
////////////////////////////////////////////////////////////////////
LOrientation OrientedParticleFactory::
get_initial_orientation() const {
  return _initial_orientation;
}

////////////////////////////////////////////////////////////////////
//     Function: get_final_orientation
//       Access: Public
////////////////////////////////////////////////////////////////////
LOrientation OrientedParticleFactory::
get_final_orientation() const {
  return _final_orientation;
}
