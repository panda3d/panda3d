// Filename: colorAttribute.h
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

#ifndef COLORATTRIBUTE_H
#define COLORATTRIBUTE_H

#include <pandabase.h>

#include "colorProperty.h"

#include <onOffAttribute.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorAttribute
// Description : See ColorTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorAttribute : public OnOffAttribute {
PUBLISHED:
  INLINE ColorAttribute();

  INLINE void set_on(const Colorf &color);
  INLINE void set_on(float r, float g, float b, float a);
  INLINE void set_uncolor();

  INLINE bool is_real() const;
  INLINE Colorf get_color() const;

public:
  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  ColorProperty _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffAttribute::init_type();
    register_type(_type_handle, "ColorAttribute",
                  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "colorAttribute.I"

#endif
