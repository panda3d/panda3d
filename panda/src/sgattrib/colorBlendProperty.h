// Filename: colorBlendProperty.h
// Created by:  drose (22Mar00)
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

#ifndef COLORBLENDPROPERTY_H
#define COLORBLENDPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorBlendProperty
// Description : This defines the types of color blending we can
//               perform.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorBlendProperty {
public:
  enum Mode {
    M_none,             // Blending is disabled
    M_multiply,         // color already in fbuffer * incoming color
    M_add,              // color already in fbuffer + incoming color
    M_multiply_add,     // color already in fbuffer * incoming color +
                        //   color already in fbuffer
    M_alpha,            // ????
  };

public:
  INLINE ColorBlendProperty(Mode mode);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const ColorBlendProperty &other) const;
  void output(ostream &out) const;

private:
  Mode _mode;
};

ostream &operator << (ostream &out, ColorBlendProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const ColorBlendProperty &prop) {
  prop.output(out);
  return out;
}

#include "colorBlendProperty.I"

#endif
