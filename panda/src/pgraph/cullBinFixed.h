// Filename: cullBinFixed.h
// Created by:  drose (29May02)
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

#ifndef CULLBINFIXED_H
#define CULLBINFIXED_H

#include "pandabase.h"

#include "cullBin.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinFixed
// Description : A specific kind of CullBin that sorts geometry in
//               the order specified by the user-specified draw_order
//               parameter.  This allows precise relative ordering of
//               two objects.
//
//               When two or more objects are assigned the same
//               draw_order, they are drawn in scene-graph order (as
//               with CullBinUnsorted).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBinFixed : public CullBin {
public:
  INLINE CullBinFixed(const string &name, GraphicsStateGuardianBase *gsg);
  virtual ~CullBinFixed();

  virtual void add_object(CullableObject *object);
  virtual void finish_cull();
  virtual void draw();

private:
  class ObjectData {
  public:
    INLINE ObjectData(CullableObject *object, int draw_order);
    INLINE bool operator < (const ObjectData &other) const;
    
    CullableObject *_object;
    int _draw_order;
  };

  typedef pvector<ObjectData> Objects;
  Objects _objects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinFixed",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBinFixed.I"

#endif


  
