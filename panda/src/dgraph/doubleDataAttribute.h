// Filename: doubleDataAttribute.h
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

#ifndef DOUBLEDATAATTRIBUTE_H
#define DOUBLEDATAATTRIBUTE_H

#include <pandabase.h>

#include "numericDataAttribute.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, NumericDataAttribute<double>);

////////////////////////////////////////////////////////////////////
//       Class : DoubleDataAttribute
// Description : A NumericDataAttribute templated on double types.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DoubleDataAttribute :
  public NumericDataAttribute<double> {
public:
  INLINE DoubleDataAttribute();
  INLINE DoubleDataAttribute(double value);

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
    NumericDataAttribute<double>::init_type();
    register_type(_type_handle, "DoubleDataAttribute",
                  NumericDataAttribute<double>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "doubleDataAttribute.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
