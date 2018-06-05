/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSphereShape.h
 * @author enn0x
 * @date 2010-01-23
 */

#ifndef __BULLET_SPHERE_SHAPE_H__
#define __BULLET_SPHERE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "collisionSphere.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletSphereShape : public BulletShape {
private:
  // Only used by make_from_bam
  INLINE BulletSphereShape() : _shape(nullptr) {};

PUBLISHED:
  explicit BulletSphereShape(PN_stdfloat radius);
  BulletSphereShape(const BulletSphereShape &copy);
  INLINE ~BulletSphereShape();

  INLINE PN_stdfloat get_radius() const;

  static BulletSphereShape *make_from_solid(const CollisionSphere *solid);

  MAKE_PROPERTY(radius, get_radius);

public:
  virtual btCollisionShape *ptr() const;

private:
  btSphereShape *_shape;
  PN_stdfloat _radius;

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
    register_type(_type_handle, "BulletSphereShape",
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

#include "bulletSphereShape.I"

#endif // __BULLET_SPHERE_SHAPE_H__
