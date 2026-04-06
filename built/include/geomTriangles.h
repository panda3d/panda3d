/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTriangles.h
 * @author drose
 * @date 2005-03-06
 */

#ifndef GEOMTRIANGLES_H
#define GEOMTRIANGLES_H

#include "pandabase.h"
#include "geomPrimitive.h"

/**
 * Defines a series of disconnected triangles.
 */
class EXPCL_PANDA_GOBJ GeomTriangles : public GeomPrimitive {
PUBLISHED:
  explicit GeomTriangles(UsageHint usage_hint);
  GeomTriangles(const GeomTriangles &copy);
  virtual ~GeomTriangles();
  ALLOC_DELETED_CHAIN(GeomTriangles);

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;

  CPT(GeomPrimitive) make_adjacency() const;

  virtual int get_num_vertices_per_primitive() const;

public:
  virtual bool draw(GraphicsStateGuardianBase *gsg,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) const;

protected:
  virtual CPT(GeomPrimitive) doubleside_impl() const;
  virtual CPT(GeomPrimitive) reverse_impl() const;
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
    register_type(_type_handle, "GeomTriangles",
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
