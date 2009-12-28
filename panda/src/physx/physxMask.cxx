// Filename: physxMask32.cxx
// Created by:  enn0x (21Oct09)
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

#include "physxMask.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxMask::all_on
//       Access: Published
//  Description: Returns a PhysxMask whose bits are all on.
////////////////////////////////////////////////////////////////////
PhysxMask PhysxMask::
all_on() {

  PhysxMask mask;
  mask._mask = 0xffffffff;
  return mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMask::all_off
//       Access: Published
//  Description: Returns a PhysxMask whose bits are all off.
////////////////////////////////////////////////////////////////////
PhysxMask PhysxMask::
all_off() {

  PhysxMask mask;
  mask._mask = 0x0000000;
  return mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMask::set_bit
//       Access: Published
//  Description: Sets the nth bit on.
//               Index must be in the range [0, 31].
////////////////////////////////////////////////////////////////////
void PhysxMask::
set_bit(unsigned int idx) {

  nassertv_always(idx >= 0 && idx <= 31);
  _mask = _mask | (1 << idx);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMask::clear_bit
//       Access: Published
//  Description: Sets the nth bit off.
//               Index must be in the range [0, 31].
////////////////////////////////////////////////////////////////////
void PhysxMask::
clear_bit(unsigned int idx) {

  nassertv_always(idx >= 0 && idx <= 31);
  _mask = _mask & ~(1 << idx);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMask::get_bit
//       Access: Published
//  Description: Returns true if the nth bit is set, false if it is
//               cleared.
//               Index must be in the range [0, 31].
////////////////////////////////////////////////////////////////////
bool PhysxMask::
get_bit(unsigned int idx) const {

  nassertr_always(idx >= 0 && idx <= 31, false);
  return (_mask & (1 << idx)) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMask::output
//       Access: Published
//  Description: Writes the PhysxMask out as a list of ones and
//               zeros.
////////////////////////////////////////////////////////////////////
void PhysxMask::
output(ostream &out) const {

  string name;

  for (int i=0; i<32; i++) {
    name += (_mask & (1 << i)) ? '1' : '0';
  }

  out << "/" << name << "/";
}

