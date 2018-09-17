/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMultiSphereShape.h
 * @author enn0x
 * @date 2012-01-04
 */

#ifndef __BULLET_MULTI_SPHERE_SHAPE_H__
#define __BULLET_MULTI_SPHERE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletShape.h"

#include "factoryParams.h"
#include "pta_LVecBase3.h"
#include "pta_stdfloat.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletMultiSphereShape : public BulletShape {
private:
  BulletMultiSphereShape() : _shape(nullptr) {}

PUBLISHED:
  explicit BulletMultiSphereShape(const PTA_LVecBase3 &points, const PTA_stdfloat &radii);
  BulletMultiSphereShape(const BulletMultiSphereShape &copy);
  void operator = (const BulletMultiSphereShape &copy);
  INLINE ~BulletMultiSphereShape();

  int get_sphere_count() const;
  LPoint3 get_sphere_pos(int index) const;
  PN_stdfloat get_sphere_radius(int index) const;

  MAKE_PROPERTY(sphere_count, get_sphere_count);
  MAKE_SEQ_PROPERTY(sphere_pos, get_sphere_count, get_sphere_pos);
  MAKE_SEQ_PROPERTY(sphere_radius, get_sphere_count, get_sphere_radius);

public:
  virtual btCollisionShape *ptr() const;

private:
  btMultiSphereShape *_shape;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletMultiSphereShape",
                  BulletShape::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletMultiSphereShape.I"

#endif // __BULLET_MULTI_SPHERE_SHAPE_H__
