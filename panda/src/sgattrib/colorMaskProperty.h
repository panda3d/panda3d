// Filename: colorMaskProperty.h
// Created by:  drose (23Mar00)
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

#ifndef COLORMASKPROPERTY_H
#define COLORMASKPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorMaskProperty
// Description : This defines the set of color planes that may be
//               active for writing to the color buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMaskProperty {
public:
  enum Mask {
    M_r = 0x001,
    M_g = 0x002,
    M_b = 0x004,
    M_a = 0x008
  };

  INLINE ColorMaskProperty(int mask);
  INLINE static ColorMaskProperty all_on();

  INLINE void set_mask(int mask);
  INLINE int get_mask() const;

  INLINE int compare_to(const ColorMaskProperty &other) const;
  void output(ostream &out) const;

private:
  int _mask;
};

INLINE ostream &operator << (ostream &out, const ColorMaskProperty &prop) {
  prop.output(out);
  return out;
}

#include "colorMaskProperty.I"

#endif
