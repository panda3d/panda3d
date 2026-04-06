/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLines.h
 * @author drose
 * @date 2005-03-22
 */

#ifndef GEOMLINES_H
#define GEOMLINES_H

#include "pandabase.h"
#include "geomPrimitive.h"

/**
 * Defines a series of disconnected line segments.
 */
class EXPCL_PANDA_GOBJ GeomLines : public GeomPrimitive {
PUBLISHED:
  explicit GeomLines(UsageHint usage_hint);
  GeomLines(const GeomLines &copy);
  virtual ~GeomLines();
  ALLOC_DELETED_CHAIN(GeomLines);

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;

  CPT(GeomPrimitive) make_adjacency() const;

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;

public:
  virtual bool draw(GraphicsStateGuardianBase *gsg,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) const;

protected:
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
    register_type(_type_handle, "GeomLines",
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
