/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTrifans.h
 * @author drose
 * @date 2005-03-08
 */

#ifndef GEOMTRIFANS_H
#define GEOMTRIFANS_H

#include "pandabase.h"
#include "geomPrimitive.h"

/**
 * Defines a series of triangle fans.
 */
class EXPCL_PANDA_GOBJ GeomTrifans : public GeomPrimitive {
PUBLISHED:
  explicit GeomTrifans(UsageHint usage_hint);
  GeomTrifans(const GeomTrifans &copy);
  virtual ~GeomTrifans();
  ALLOC_DELETED_CHAIN(GeomTrifans);

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;
  virtual int get_geom_rendering() const;

public:
  virtual bool draw(GraphicsStateGuardianBase *gsg,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) const;

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
