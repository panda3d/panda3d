// Filename: alphaTransformProperty.h
// Created by:  jason (01Aug00)
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

#ifndef ALPHA_TRANSFORM_PROPERTY_H
#define ALPHA_TRANSFORM_PROPERTY_H

#include <pandabase.h>

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : AlphaTransformProperty
// Description : This class defines the set state for polygon offseting
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AlphaTransformProperty {
public:
  INLINE AlphaTransformProperty();
  INLINE AlphaTransformProperty(float offset, float scale);

  INLINE void set_offset(float offset);
  INLINE float get_offset() const;
  INLINE void set_scale(float scale);
  INLINE float get_scale() const;

  int compare_to(const AlphaTransformProperty &other) const;
  void output(ostream &out) const;

private:
  float _offset;
  float _scale;
};

INLINE ostream &operator << (ostream &out, const AlphaTransformProperty &prop) {
  prop.output(out);
  return out;
}

#include "alphaTransformProperty.I"

#endif
