// Filename: vec4DataAttribute.h
// Created by:  jason (03Aug00)
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

#ifndef VEC4DATAATTRIBUTE_H
#define VEC4DATAATTRIBUTE_H

#include <pandabase.h>

#include "vectorDataAttribute.h"

#include <luse.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define VECTORDATAATTRIBUTE_LPOINT4F VectorDataAttribute<LPoint4f, LMatrix4f>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VECTORDATAATTRIBUTE_LPOINT4F);

////////////////////////////////////////////////////////////////////
//       Class : Vec4DataAttribute
// Description : A VectorDataAttribute templated on LPoint4f.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Vec4DataAttribute :
  public VectorDataAttribute<LPoint4f, LMatrix4f> {
public:
  INLINE Vec4DataAttribute();
  INLINE Vec4DataAttribute(const LVecBase4f &value);

  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VectorDataAttribute<LPoint4f, LMatrix4f>::init_type();
    register_type(_type_handle, "Vec4DataAttribute",
                  VectorDataAttribute<LPoint4f, LMatrix4f>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "vec4DataAttribute.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
