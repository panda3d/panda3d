// Filename: polygonOffsetProperty.h
// Created by:  jason (12Jul00)
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

#ifndef POLYGONOFFSETPROPERTY_H
#define POLYGONOFFSETPROPERTY_H

#include <pandabase.h>

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : PolygonOffsetProperty
// Description : This class defines the set state for polygon offseting
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolygonOffsetProperty {
public:
  INLINE PolygonOffsetProperty();
  INLINE PolygonOffsetProperty(int units, int factor);

  INLINE void set_units(int units);
  INLINE int get_units() const;
  INLINE void set_factor(int factor);
  INLINE int get_factor() const;

  INLINE int compare_to(const PolygonOffsetProperty &other) const;
  void output(ostream &out) const;

private:
  int _units;
  int _factor;
};

INLINE ostream &operator << (ostream &out, const PolygonOffsetProperty &prop) {
  prop.output(out);
  return out;
}

#include "polygonOffsetProperty.I"

#endif
