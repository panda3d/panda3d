// Filename: collisionTube.h
// Created by:  drose (25Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONTUBE_H
#define COLLISIONTUBE_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "parabola.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionTube
// Description : This implements a solid roughly in cylindrical shape.
//               It's not called a CollisionCylinder because it's not
//               a true cylinder; specifically, it has rounded ends
//               instead of flat ends.  It looks more like a Contac
//               pill.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionTube : public CollisionSolid {
PUBLISHED:
  INLINE CollisionTube(const LPoint3 &a, const LPoint3 &db,
                       PN_stdfloat radius);
  INLINE CollisionTube(PN_stdfloat ax, PN_stdfloat ay, PN_stdfloat az,
                       PN_stdfloat bx, PN_stdfloat by, PN_stdfloat bz,
                       PN_stdfloat radius);

  virtual LPoint3 get_collision_origin() const;

private:
  INLINE CollisionTube();

public:
  INLINE CollisionTube(const CollisionTube &copy);
  virtual CollisionSolid *make_copy();

  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(ostream &out) const;

  INLINE static void flush_level();

PUBLISHED:
  INLINE void set_point_a(const LPoint3 &a);
  INLINE void set_point_a(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_point_a() const;

  INLINE void set_point_b(const LPoint3 &b);
  INLINE void set_point_b(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_point_b() const;

  INLINE void set_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_radius() const;

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

protected:
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_line(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_parabola(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  void recalc_internals();

  LVertex calc_sphere1_vertex(int ri, int si, int num_rings, int num_slices);
  LVertex calc_sphere2_vertex(int ri, int si, int num_rings, int num_slices,
                              PN_stdfloat length);

  bool intersects_line(double &t1, double &t2,
                       const LPoint3 &from, const LVector3 &delta,
                       PN_stdfloat inflate_radius) const;
  bool sphere_intersects_line(double &t1, double &t2, PN_stdfloat center_y,
                              const LPoint3 &from, const LVector3 &delta,
                              PN_stdfloat inflate_radius) const;
  bool intersects_parabola(double &t, const LParabola &parabola,
                           double t1, double t2,
                           const LPoint3 &p1, const LPoint3 &p2) const;
  void calculate_surface_point_and_normal(const LPoint3 &surface_point,
                                          double extra_radius,
                                          LPoint3 &result_point,
                                          LVector3 &result_normal) const;
  void set_intersection_point(CollisionEntry *new_entry, 
                              const LPoint3 &into_intersection_point, 
                              double extra_radius) const;

private:
  LPoint3 _a, _b;
  PN_stdfloat _radius;

  // These are derived from the above.
  LMatrix4 _mat;
  LMatrix4 _inv_mat;
  PN_stdfloat _length;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

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


