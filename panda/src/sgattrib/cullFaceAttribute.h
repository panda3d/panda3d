// Filename: cullFaceAttribute.h
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

#ifndef CULLFACEATTRIBUTE_H
#define CULLFACEATTRIBUTE_H

#include <pandabase.h>

#include "cullFaceProperty.h"

#include <onAttribute.h>

////////////////////////////////////////////////////////////////////
//       Class : CullFaceAttribute
// Description : See CullFaceTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullFaceAttribute : public OnAttribute {
public:
  INLINE CullFaceAttribute();

  INLINE void set_mode(CullFaceProperty::Mode);
  INLINE CullFaceProperty::Mode get_mode() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  CullFaceProperty _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnAttribute::init_type();
    register_type(_type_handle, "CullFaceAttribute",
                  OnAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "cullFaceAttribute.I"

#endif
