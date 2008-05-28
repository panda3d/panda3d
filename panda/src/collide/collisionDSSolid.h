// Filename: CollisionDSSolid.h
// Created by:  Dave Schuyler (05Apr06)
// Based on collision tube by:  drose
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

#ifndef COLLISIONDSSOLID_H
#define COLLISIONDSSOLID_H

#include "pandabase.h"
#include "plane.h"
#include "collisionSolid.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionDSSolid
// Description : A collision volume or object made up of the intersection
//               of two spheres (potentially a lens) and two half-spaces
//               (planes).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionDSSolid: public CollisionSolid {
PUBLISHED:
  INLINE CollisionDSSolid(
    const LPoint3f &center_a, float radius_a,
    const LPoint3f &center_b, float radius_b,
    const Planef &plane_a, const Planef &plane_b);
  INLINE CollisionDSSolid(
    float ax, float ay, float az, float radius_a,
    float bx, float by, float bz, float radius_b,
    const Planef &plane_a, const Planef &plane_b);

  virtual LPoint3f get_collision_origin() const;

protected:
  INLINE CollisionDSSolid();

public:
  INLINE CollisionDSSolid(const CollisionDSSolid &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  INLINE float get_lens_radius() const;

  virtual void xform(const LMatrix4f &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(ostream &out) const;

PUBLISHED:
  // Sphere a
  INLINE void set_center_a(const LPoint3f &center);
  INLINE void set_center_a(float x, float y, float z);
  INLINE const LPoint3f &get_center_a() const;

  INLINE void set_radius_a(float radius);
  INLINE float get_radius_a() const;

  // Sphere b
  INLINE void set_center_b(const LPoint3f &center);
  INLINE void set_center_b(float x, float y, float z);
  INLINE const LPoint3f &get_center_b() const;

  INLINE void set_radius_b(float radius);
  INLINE float get_radius_b() const;

  // Plane a
  INLINE LVector3f get_normal_a() const;
  INLINE float dist_to_plane_a(const LPoint3f &point) const;

  INLINE void set_plane_a(const Planef &plane);
  INLINE const Planef &get_plane_a() const;

  // Plane b
  INLINE LVector3f get_normal_b() const;
  INLINE float dist_to_plane_b(const LPoint3f &point) const;

  INLINE void set_plane_b(const Planef &plane);
  INLINE const Planef &get_plane_b() const;

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

protected:
  void recalc_internals();
  Vertexf compute_point(float latitude, float longitude) const;

private:
  LPoint3f _center_a;
  float _radius_a;
  LPoint3f _center_b;
  float _radius_b;
  Planef _plane_a;
  Planef _plane_b;
  
  mutable float _lens_radius;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

  void calc_plane(const Planef &plane);
  Vertexf calc_sphere1_vertex(
      int ri, int si, int num_rings, int num_slices, float length,
      float angle);
  Vertexf calc_sphere2_vertex(
      int ri, int si, int num_rings, int num_slices, float length,
      float angle);

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_CollisionSphere(const FactoryParams &params);
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionDSSolid",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionDSSolid.I"

#endif
