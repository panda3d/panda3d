// Filename: physxMask32.h
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

#ifndef PHYSXMASK_H
#define PHYSXMASK_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMask
// Description : 32-bit bitmask class.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMask {

PUBLISHED:
  INLINE PhysxMask();
  INLINE ~PhysxMask();

  void set_bit(unsigned int idx);
  void clear_bit(unsigned int idx);
  bool get_bit(unsigned int idx) const;

  void output(ostream &out) const;

  static PhysxMask all_on();
  static PhysxMask all_off();

public:
  INLINE NxU32 get_mask() const;

private:
  NxU32 _mask;
};

INLINE ostream &operator << (ostream &out, const PhysxMask &mask) {
  mask.output(out);
  return out;
}

#include "physxMask.I"

#endif // PHYSXMASK_H
