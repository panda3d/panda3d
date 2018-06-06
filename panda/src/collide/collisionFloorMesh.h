/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionFloorMesh.h
 * @author zpavlov
 * @date 2007-06-28
 */

#ifndef COLLISIONFLOORMESH_H
#define COLLISIONFLOORMESH_H

#include "pandabase.h"

#include "collisionPlane.h"
#include "clipPlaneAttrib.h"
#include "look_at.h"
#include "pvector.h"

class GeomNode;

/**
 * This object represents a solid made entirely of triangles, which will only
 * be tested again z axis aligned rays
 */
class EXPCL_PANDA_COLLIDE CollisionFloorMesh : public CollisionSolid {
public:
  typedef struct {
    unsigned int p1;
    unsigned int p2;
    unsigned int p3;
    PN_stdfloat min_x;
    PN_stdfloat max_x;
    PN_stdfloat min_y;
    PN_stdfloat max_y;
  } TriangleIndices;

PUBLISHED:
  INLINE CollisionFloorMesh();

  INLINE void add_vertex(const LPoint3 &vert);
  void add_triangle(unsigned int pointA, unsigned int pointB, unsigned int pointC);

  INLINE unsigned int get_num_vertices() const;
  INLINE const LPoint3 &get_vertex(unsigned int index) const;
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);
  INLINE unsigned int get_num_triangles() const;
  INLINE LPoint3i get_triangle(unsigned int index) const;
  MAKE_SEQ(get_triangles, get_num_triangles, get_triangle);

  virtual LPoint3 get_collision_origin() const;

PUBLISHED:
  MAKE_SEQ_PROPERTY(vertices, get_num_vertices, get_vertex);
  MAKE_SEQ_PROPERTY(triangles, get_num_triangles, get_triangle);

public:
  CollisionFloorMesh(const CollisionFloorMesh &copy);
  virtual CollisionSolid *make_copy();

public:

  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  INLINE static void flush_level();

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

  virtual PT(CollisionEntry)
    test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
    test_intersection_from_sphere(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  typedef pvector<LPoint3> Vertices;
  typedef pvector<TriangleIndices> Triangles;

  Vertices _vertices;
  Triangles _triangles;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_CollisionFloorMesh(const FactoryParams &params);
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionFloorMesh",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionFloorMesh.I"

#endif
