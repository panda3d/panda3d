// Filename: matrixDataAttribute.h
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

#ifndef MATRIXDATAATTRIBUTE_H
#define MATRIXDATAATTRIBUTE_H

#include <pandabase.h>

#include "vectorDataAttribute.h"

#include <luse.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define VECTORDATAATTRIBUTE_LMATRIX4F VectorDataAttribute<LMatrix4f, LMatrix4f>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VECTORDATAATTRIBUTE_LMATRIX4F);

////////////////////////////////////////////////////////////////////
//       Class : MatrixDataAttribute
// Description : A VectorDataAttribute templated on LMatrix4f.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MatrixDataAttribute :
  public VectorDataAttribute<LMatrix4f, LMatrix4f> {
public:
  INLINE MatrixDataAttribute();
  INLINE MatrixDataAttribute(const LMatrix4f &value);

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
    VectorDataAttribute<LMatrix4f, LMatrix4f>::init_type();
    register_type(_type_handle, "MatrixDataAttribute",
                  VectorDataAttribute<LMatrix4f, LMatrix4f>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "matrixDataAttribute.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
