/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMask.cxx
 * @author enn0x
 * @date 2009-10-21
 */

#include "physxMask.h"

/**
 * Returns a PhysxMask whose bits are all on.
 */
PhysxMask PhysxMask::
all_on() {

  PhysxMask mask;
  mask._mask = 0xffffffff;
  return mask;
}

/**
 * Returns a PhysxMask whose bits are all off.
 */
PhysxMask PhysxMask::
all_off() {

  PhysxMask mask;
  mask._mask = 0x0000000;
  return mask;
}

/**
 * Sets the nth bit on.  Index must be in the range [0, 31].
 */
void PhysxMask::
set_bit(unsigned int idx) {

  nassertv_always(idx >= 0 && idx <= 31);
  _mask = _mask | (1 << idx);
}

/**
 * Sets the nth bit off.  Index must be in the range [0, 31].
 */
void PhysxMask::
clear_bit(unsigned int idx) {

  nassertv_always(idx >= 0 && idx <= 31);
  _mask = _mask & ~(1 << idx);
}

/**
 * Returns true if the nth bit is set, false if it is cleared.  Index must be
 * in the range [0, 31].
 */
bool PhysxMask::
get_bit(unsigned int idx) const {

  nassertr_always(idx >= 0 && idx <= 31, false);
  return (_mask & (1 << idx)) ? true : false;
}

/**
 * Writes the PhysxMask out as a list of ones and zeros.
 */
void PhysxMask::
output(std::ostream &out) const {

  std::string name;

  for (int i=0; i<32; i++) {
    name += (_mask & (1 << i)) ? '1' : '0';
  }

  out << "/" << name << "/";
}
