// Filename: factoryParam.h
// Created by:  drose (08May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef FACTORYPARAM_H
#define FACTORYPARAM_H

#include "pandabase.h"

#include "typedReferenceCount.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : FactoryParam
// Description : The base class of any number of specific pieces of
//               parameter information that might be passed to a
//               Factory's CreateFunc to control what kind of instance
//               is created.  This class is empty and contains no
//               data, but different kinds of factories may expect
//               parameters of various types that derive from
//               FactoryParam (and do contain data).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FactoryParam : public TypedReferenceCount {
public:
  INLINE FactoryParam();
  INLINE FactoryParam(const FactoryParam &other);
  INLINE void operator = (const FactoryParam &other);
  INLINE ~FactoryParam();

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "FactoryParam",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "factoryParam.I"

#endif

