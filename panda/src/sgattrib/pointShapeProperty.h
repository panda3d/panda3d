// Filename: pointShapeProperty.h
// Created by:  charles (10Jul00)
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

#ifndef POINTSHAPEPROPERTY_H
#define POINTSHAPEPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : PointShapeProperty
// Description : Affects the shapes of point primitives
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PointShapeProperty {
public:

  enum Mode {
    M_square,
    M_circle
  };

public:

  INLINE PointShapeProperty(Mode mode);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode(void) const;

  INLINE int compare_to(const PointShapeProperty& other) const;
  void output(ostream &out) const;

private:

  Mode _mode;
};

ostream &operator << (ostream &out, PointShapeProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const PointShapeProperty &prop) {
  prop.output(out);
  return out;
}

#include "pointShapeProperty.I"

#endif // POINTSHAPEPROPERTY_H
