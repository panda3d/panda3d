// Filename: geomTrifans.h
// Created by:  drose (08Mar05)
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

#ifndef GEOMTRIFANS_H
#define GEOMTRIFANS_H

#include "pandabase.h"
#include "geomPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomTrifans
// Description : Defines a series of triangle fans.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomTrifans : public GeomPrimitive {
PUBLISHED:
  GeomTrifans(UsageHint usage_hint);
  GeomTrifans(const GeomTrifans &copy);
  virtual ~GeomTrifans();

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;
  virtual int get_geom_rendering() const;

public:
  virtual void draw(GraphicsStateGuardianBase *gsg) const;

protected:
  virtual CPT(GeomPrimitive) decompose_impl() const;
  virtual CPT(GeomVertexArrayData) rotate_impl() const;

public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomPrimitive::init_type();
    register_type(_type_handle, "GeomTrifans",
                  GeomPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Geom;
};

#endif
