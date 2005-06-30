// Filename: geomTristrips.h
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

#ifndef GEOMTRISTRIPS_H
#define GEOMTRISTRIPS_H

#include "pandabase.h"
#include "geomPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomTristrips
// Description : Defines a series of triangle strips.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomTristrips : public GeomPrimitive {
PUBLISHED:
  GeomTristrips(UsageHint usage_hint);
  GeomTristrips(const GeomTristrips &copy);
  virtual ~GeomTristrips();

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;
  virtual int get_geom_rendering() const;
  virtual int get_num_unused_vertices_per_primitive() const;

public:
  virtual void draw(GraphicsStateGuardianBase *gsg) const;

protected:
  virtual CPT(GeomPrimitive) decompose_impl() const;
  virtual CPT(GeomVertexArrayData) rotate_impl() const;
  virtual bool requires_unused_vertices() const;
  virtual void append_unused_vertices(GeomVertexArrayData *vertices, 
                                      int vertex);

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
    register_type(_type_handle, "GeomTristrips",
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
