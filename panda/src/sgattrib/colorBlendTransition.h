// Filename: colorBlendTransition.h
// Created by:  mike (18Jan99)
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

#ifndef COLORBLENDTRANSITION_H
#define COLORBLENDTRANSITION_H

#include <pandabase.h>

#include "colorBlendProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorBlendTransition
// Description : This controls the kinds of blending between colors
//               being rendered and the existing frame buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorBlendTransition : public OnTransition {
public:
  INLINE ColorBlendTransition(ColorBlendProperty::Mode mode = ColorBlendProperty::M_none);

  INLINE void set_mode(ColorBlendProperty::Mode mode);
  INLINE ColorBlendProperty::Mode get_mode() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  ColorBlendProperty _value;
  static PT(NodeTransition) _initial;

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
    register_type(_type_handle, "ColorBlendTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "colorBlendTransition.I"

#endif
