// Filename: textureApplyAttribute.h
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

#ifndef TEXTUREAPPLYATTRIBUTE_H
#define TEXTUREAPPLYATTRIBUTE_H

#include <pandabase.h>

#include "textureApplyProperty.h"

#include <onAttribute.h>

////////////////////////////////////////////////////////////////////
//       Class : TextureApplyAttribute
// Description : See TextureApplyTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureApplyAttribute : public OnAttribute {
public:
  INLINE TextureApplyAttribute();

  INLINE void set_mode(TextureApplyProperty::Mode mode);
  INLINE TextureApplyProperty::Mode get_mode() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  TextureApplyProperty _value;

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
    register_type(_type_handle, "TextureApplyAttribute",
                  OnAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "textureApplyAttribute.I"

#endif
