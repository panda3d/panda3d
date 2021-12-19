/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLinesAdjacency.h
 * @author rdb
 * @date 2018-03-01
 */

#ifndef GEOMLINESADJACENCY_H
#define GEOMLINESADJACENCY_H

#include "pandabase.h"
#include "geomPrimitive.h"

/**
 * Defines a series of disconnected line segments with adjacency information,
 * for use with geometry shaders.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_GOBJ GeomLinesAdjacency : public GeomPrimitive {
PUBLISHED:
  explicit GeomLinesAdjacency(UsageHint usage_hint);
  GeomLinesAdjacency(const GeomLinesAdjacency &copy);
  virtual ~GeomLinesAdjacency();
  ALLOC_DELETED_CHAIN(GeomLinesAdjacency);

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;
  virtual int get_geom_rendering() const;

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;

public:
  virtual bool draw(GraphicsStateGuardianBase *gsg,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) const;

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
    register_type(_type_handle, "GeomLinesAdjacency",
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
