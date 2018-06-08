/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConvexPointCloudShape.h
 * @author enn0x
 * @date 2010-01-30
 */

#ifndef __BULLET_CONVEX_POINT_CLOUD_SHAPE_H__
#define __BULLET_CONVEX_POINT_CLOUD_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletShape.h"

#include "geom.h"
#include "pta_LVecBase3.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletConvexPointCloudShape : public BulletShape {
private:
  // Only used by make_from_bam
  INLINE BulletConvexPointCloudShape();

PUBLISHED:
  explicit BulletConvexPointCloudShape(const PTA_LVecBase3 &points, LVecBase3 scale=LVecBase3(1.));
  explicit BulletConvexPointCloudShape(const Geom *geom, LVecBase3 scale=LVecBase3(1.));
  BulletConvexPointCloudShape(const BulletConvexPointCloudShape &copy);
  INLINE ~BulletConvexPointCloudShape();

  int get_num_points() const;

  MAKE_PROPERTY(num_points, get_num_points);

public:
  virtual btCollisionShape *ptr() const;

private:
  btConvexPointCloudShape *_shape;
  LVecBase3 _scale;

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
    register_type(_type_handle, "BulletConvexPointCloudShape",
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

#include "bulletConvexPointCloudShape.I"

#endif // __BULLET_CONVEX_POINT_CLOUD_SHAPE_H__
