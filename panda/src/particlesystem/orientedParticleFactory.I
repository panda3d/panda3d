// Filename: orientedParticleFactory.I
// Created by:  charles (16Aug00)
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

////////////////////////////////////////////////////////////////////
// Function : set_initial_orientation
//   Access : public
////////////////////////////////////////////////////////////////////
void OrientedParticleFactory::
set_initial_orientation(const LOrientationf &o) {
  _initial_orientation = o;
}

////////////////////////////////////////////////////////////////////
// Function : set_final_orientation
//   Access : public
////////////////////////////////////////////////////////////////////
void OrientedParticleFactory::
set_final_orientation(const LOrientationf &o) {
  _final_orientation = o;
}

////////////////////////////////////////////////////////////////////
// Function : get_initial_orientation
//   Access : public
////////////////////////////////////////////////////////////////////
LOrientationf OrientedParticleFactory::
get_initial_orientation() const {
  return _initial_orientation;
}

////////////////////////////////////////////////////////////////////
// Function : get_final_orientation
//   Access : public
////////////////////////////////////////////////////////////////////
LOrientationf OrientedParticleFactory::
get_final_orientation() const {
  return _final_orientation;
}
