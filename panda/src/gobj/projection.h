// Filename: projection.h
// Created by:  drose (18Feb99)
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

#ifndef PROJECTION_H
#define PROJECTION_H

#include <pandabase.h>

#include <typedReferenceCount.h>
#include <lmatrix.h>
#include "geom.h"

class BoundingVolume;

////////////////////////////////////////////////////////////////////
//       Class : Projection
// Description : A base class for any number of different kinds of
//               linear projections.  Presently, this includes
//               perspective and orthographic projections.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Projection : public TypedReferenceCount {
PUBLISHED:
  virtual Projection *make_copy() const=0;

  virtual bool extrude(const LPoint2f &point2d,
                       LPoint3f &origin, LVector3f &direction,
                       CoordinateSystem cs = CS_default) const;
  virtual bool project(const LPoint3f &point3d, LPoint2f &point2d,
                       CoordinateSystem cs = CS_default) const;

public:
  virtual LMatrix4f get_projection_mat(CoordinateSystem cs = CS_default) const=0;
  virtual Geom* make_geometry(const Colorf &color = Colorf(0.0, 1.0, 0.0, 1.0),
                              CoordinateSystem cs = CS_default) const;

  virtual BoundingVolume *make_bounds(CoordinateSystem cs = CS_default) const;

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
    register_type(_type_handle, "Projection",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

