/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMeshShape.h
 * @author enn0x
 * @date 2010-02-09
 */

#ifndef __BULLET_TRIANGLE_MESH_SHAPE_H__
#define __BULLET_TRIANGLE_MESH_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletShape.h"

#include "factoryParams.h"
#include "luse.h"

class BulletTriangleMesh;

/**
 *
 */
class EXPCL_PANDABULLET BulletTriangleMeshShape : public BulletShape {
private:
  INLINE BulletTriangleMeshShape();

PUBLISHED:
  explicit BulletTriangleMeshShape(BulletTriangleMesh *mesh, bool dynamic, bool compress=true, bool bvh=true);
  BulletTriangleMeshShape(const BulletTriangleMeshShape &copy);
  INLINE ~BulletTriangleMeshShape();

  void refit_tree(const LPoint3 &aabb_min, const LPoint3 &aabb_max);

  INLINE bool is_static() const;
  INLINE bool is_dynamic() const;

  MAKE_PROPERTY(static, is_static);
  MAKE_PROPERTY(dynamic, is_dynamic);

public:
  virtual btCollisionShape *ptr() const;

private:
  btBvhTriangleMeshShape *_bvh_shape;
  btGImpactMeshShape *_gimpact_shape;

  PT(BulletTriangleMesh) _mesh;

  // Stored temporarily during bam read.
  PN_stdfloat _margin;

  bool _dynamic : 1;
  bool _compress : 1;
  bool _bvh : 1;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                              BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletTriangleMeshShape",
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

#include "bulletTriangleMeshShape.I"

#endif // __BULLET_TRIANGLE_MESH_SHAPE_H__
