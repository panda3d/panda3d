// Filename: pointShapeTransition.h
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

#ifndef POINTSHAPETRANSITION_H
#define POINTSHAPETRANSITION_H

#include <pandabase.h>

#include "pointShapeProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : PointShapeTransition
// Description : An arc that determines the shape of point primitives
//               drawn beneath.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PointShapeTransition : public OnTransition {
public:
  INLINE PointShapeTransition(PointShapeProperty::Mode mode = PointShapeProperty::M_square);

  INLINE void set_mode(PointShapeProperty::Mode mode);
  INLINE PointShapeProperty::Mode get_mode() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PointShapeProperty _value;

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
    register_type(_type_handle, "PointShapeTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class PointShapeAttribute;
};

#include "pointShapeTransition.I"

#endif // POINTSHAPETRANSITION_H
