// Filename: vec3DataAttribute.h
// Created by:  drose (27Mar00)
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

#ifndef VEC3DATAATTRIBUTE_H
#define VEC3DATAATTRIBUTE_H

#include <pandabase.h>

#include "vectorDataAttribute.h"

#include <luse.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define VECTORDATAATTRIBUTE_LPOINT3F VectorDataAttribute<LPoint3f, LMatrix4f>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VECTORDATAATTRIBUTE_LPOINT3F);

////////////////////////////////////////////////////////////////////
//       Class : Vec3DataAttribute
// Description : A VectorDataAttribute templated on LPoint3f.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Vec3DataAttribute :
  public VectorDataAttribute<LPoint3f, LMatrix4f> {
public:
  INLINE Vec3DataAttribute();
  INLINE Vec3DataAttribute(const LVecBase3f &value);

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
    VectorDataAttribute<LPoint3f, LMatrix4f>::init_type();
    register_type(_type_handle, "Vec3DataAttribute",
                  VectorDataAttribute<LPoint3f, LMatrix4f>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "vec3DataAttribute.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
