/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletPlaneShape.h
 * @author enn0x
 * @date 2010-01-23
 */

#ifndef __BULLET_PLANE_SHAPE_H__
#define __BULLET_PLANE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "collisionPlane.h"
#include "luse.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletPlaneShape : public BulletShape {
private:
  // Only used by make_from_bam
  INLINE BulletPlaneShape() : _shape(nullptr) {};

PUBLISHED:
  explicit BulletPlaneShape(LPlane plane);
  explicit BulletPlaneShape(const LVector3 &normal, PN_stdfloat constant);
  BulletPlaneShape(const BulletPlaneShape &copy);
  INLINE ~BulletPlaneShape();

  LPlane get_plane() const;
  LVector3 get_plane_normal() const;
  PN_stdfloat get_plane_constant() const;

  static BulletPlaneShape *make_from_solid(const CollisionPlane *solid);

  MAKE_PROPERTY(plane, get_plane);
  MAKE_PROPERTY(plane_normal, get_plane_normal);
  MAKE_PROPERTY(plane_constant, get_plane_constant);

public:
  virtual btCollisionShape *ptr() const;

private:
  btStaticPlaneShape *_shape;

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
    register_type(_type_handle, "BulletPlaneShape",
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

#include "bulletPlaneShape.I"

#endif // __BULLET_PLANE_SHAPE_H__
