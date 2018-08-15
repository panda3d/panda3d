/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxGroupsMask.cxx
 * @author enn0x
 * @date 2009-10-21
 */

#include "physxGroupsMask.h"

using std::string;

/**
 * Returns a PhysxGroupsMask whose bits are all on.
 */
PhysxGroupsMask PhysxGroupsMask::
all_on() {

  PhysxGroupsMask mask;
  mask._mask.bits0 = 0xffffffff;
  mask._mask.bits1 = 0xffffffff;
  mask._mask.bits2 = 0xffffffff;
  mask._mask.bits3 = 0xffffffff;
  return mask;
}

/**
 * Returns a PhysxGroupsMask whose bits are all off.
 */
PhysxGroupsMask PhysxGroupsMask::
all_off() {

  PhysxGroupsMask mask;

  mask._mask.bits0 = 0x0000000;
  mask._mask.bits1 = 0x0000000;
  mask._mask.bits2 = 0x0000000;
  mask._mask.bits3 = 0x0000000;

  return mask;
}

/**
 * Sets the nth bit on.  Index must be in the range [0, 127].
 */
void PhysxGroupsMask::
set_bit(unsigned int idx) {

  nassertv_always(idx >= 0 && idx <= 127);

  NxU32 bits = 1 << (idx % 32);

  if (idx < 32) {
    _mask.bits0 |= bits;
  }
  else if (idx < 64) {
    _mask.bits1 |= bits;
  }
  else if (idx < 96) {
    _mask.bits2 |= bits;
  }
  else {
    _mask.bits3 |= bits;
  }
}

/**
 * Sets the nth bit off.  Index must be in the range [0, 127].
 */
void PhysxGroupsMask::
clear_bit(unsigned int idx) {

  nassertv_always(idx >= 0 && idx <= 127);

  NxU32 bits = 1 << (idx % 32);

  if (idx < 32) {
    _mask.bits0 = _mask.bits0 & ~bits;
  }
  else if (idx < 64) {
    _mask.bits1 = _mask.bits1 & ~bits;
  }
  else if (idx < 96) {
    _mask.bits2 = _mask.bits2 & ~bits;
  }
  else {
    _mask.bits3 = _mask.bits3 & ~bits;
  }
}

/**
 * Returns true if the nth bit is set, false if it is cleared.  Index must be
 * in the range [0, 127].
 */
bool PhysxGroupsMask::
get_bit(unsigned int idx) const {

  nassertr_always(idx >= 0 && idx <= 127, false);

  NxU32 bits = 1 << (idx % 32);

  if (idx < 32) {
    return (_mask.bits0 & bits) ? true : false;
  }
  else if (idx < 64) {
    return (_mask.bits1 & bits) ? true : false;
  }
  else if (idx < 96) {
    return (_mask.bits2 & bits) ? true : false;
  }
  else {
    return (_mask.bits3 & bits) ? true : false;
  }
}

/**
 * Writes the PhysxGroupsMask out as a list of ones and zeros.
 */
void PhysxGroupsMask::
output(std::ostream &out) const {

  string name0;
  string name1;
  string name2;
  string name3;

  for (int i=0; i<32; i++) {
    name0 += (_mask.bits0 & (1 << i)) ? '1' : '0';
    name1 += (_mask.bits1 & (1 << i)) ? '1' : '0';
    name2 += (_mask.bits2 & (1 << i)) ? '1' : '0';
    name3 += (_mask.bits3 & (1 << i)) ? '1' : '0';
  }

  out << "/" << name0 << "-" << name1 << "-" << name2 << "-" << name3 << "/";
}
