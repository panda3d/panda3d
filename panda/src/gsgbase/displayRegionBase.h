// Filename: displayRegionBase.h
// Created by:  drose (20Feb09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef DISPLAYREGIONBASE_H
#define DISPLAYREGIONBASE_H

#include "pandabase.h"

#include "typedReferenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegionBase
// Description : An abstract base class for DisplayRegion, mainly so
//               we can store DisplayRegion pointers in a Camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GSGBASE DisplayRegionBase : public TypedReferenceCount {
protected:
  INLINE DisplayRegionBase();

public:
  virtual ~DisplayRegionBase();

PUBLISHED:
  virtual void output(ostream &out) const=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "DisplayRegionBase",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const DisplayRegionBase &dr);

#include "displayRegionBase.I"

#endif
