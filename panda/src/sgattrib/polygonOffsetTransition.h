// Filename: polygonOffsetTransition.h
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

#ifndef POLYGONOFFSETTRANSITION_H
#define POLYGONOFFSETTRANSITION_H

#include <pandabase.h>

#include "polygonOffsetProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : PolygonOffsetTransition
// Description : This controls the amount of offseting done for depth
//               tests.  The first value is the units, this is the
//               fixed offseting to be used, and the second value
//               allows for variable offsetting per polygon based on
//               the ratio of depth relative to screen area of the polygon
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolygonOffsetTransition : public OnTransition {
public:
  INLINE PolygonOffsetTransition(int amount = 0, int factor = 0);

  INLINE void set_units(int units);
  INLINE int get_units() const;
  INLINE void set_factor(int factor);
  INLINE int get_factor() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PolygonOffsetProperty _state;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnTransition::init_type();
    register_type(_type_handle, "PolygonOffsetTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


#include "polygonOffsetTransition.I"

#endif
