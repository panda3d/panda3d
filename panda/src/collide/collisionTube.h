// Filename: collisionTube.h
// Created by:  drose (25Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONTUBE_H
#define COLLISIONTUBE_H

#include "pandabase.h"

#include "collisionSolid.h"

///////////////////////////////////////////////////////////////////
//       Class : CollisionTube
// Description : This implements a solid roughly in cylindrical shape.
//               It's not called a CollisionCylinder because it's not
//               a true cylinder; specifically, it has rounded ends
//               instead of flat ends.  It looks more like a Contac
//               pill.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionTube : public CollisionSolid {
PUBLISHED:
  INLINE CollisionTube(const LPoint3f &a, const LPoint3f &db,
                       float radius);
  INLINE CollisionTube(float ax, float ay, float az,
                       float bx, float by, float bz,
                       float radius);

private:
  INLINE CollisionTube();

public:
  INLINE CollisionTube(const CollisionTube &copy);
  virtual CollisionSolid *make_copy();

  virtual void xform(const LMatrix4f &mat);
  virtual LPoint3f get_collision_origin() const;

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_point_a(const LPoint3f &a);
  INLINE void set_point_a(float x, float y, float z);
  INLINE const LPoint3f &get_point_a() const;

  INLINE void set_point_b(const LPoint3f &b);
  INLINE void set_point_b(float x, float y, float z);
  INLINE const LPoint3f &get_point_b() const;

  INLINE void set_radius(float radius);
  INLINE float get_radius() const;

protected:
  virtual BoundingVolume *recompute_bound();

protected:
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_line(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  void recalc_internals();

  Vertexf calc_sphere1_vertex(int ri, int si, int num_rings, int num_slices);
  Vertexf calc_sphere2_vertex(int ri, int si, int num_rings, int num_slices,
                              float length);

  bool intersects_line(double &t1, double &t2,
                       LPoint3f from, LVector3f delta,
                       float inflate_radius) const;
  bool sphere_intersects_line(double &t1, double &t2, float center_y,
                              const LPoint3f &from, const LVector3f &delta,
                              float inflate_radius) const;
  void set_intersection_point(CollisionEntry *new_entry, 
                              const LPoint3f &into_intersection_point, 
                              double extra_radius) const;

private:
  LPoint3f _a, _b;
  float _radius;

  // These are derived from the above.
  LMatrix4f _mat;
  LMatrix4f _inv_mat;
  float _length;

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
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionTube",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionTube.I"

#endif


