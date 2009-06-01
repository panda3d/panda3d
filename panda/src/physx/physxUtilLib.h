// Filename: physxUtilLib.h
// Created by:  pratt (Apr 7, 2006)
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

#ifndef PHYSXUTILLIB_H
#define PHYSXUTILLIB_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxBounds3;
class PhysxBox;
class PhysxCapsule;
class PhysxJointDesc;
class PhysxPlane;
class PhysxRay;
class PhysxSegment;
class PhysxSphere;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxUtilLib
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxUtilLib {
PUBLISHED:

  bool physx_box_box_intersect(const LVecBase3f & extents0, const LVecBase3f & center0, const LMatrix3f & rotation0, const LVecBase3f & extents1, const LVecBase3f & center1, const LMatrix3f & rotation1, bool full_test);
  bool physx_box_contains_point(const PhysxBox & box, const LVecBase3f & p);
  const unsigned int * physx_box_vertex_to_quad(unsigned int vertex_index);
  bool physx_build_smooth_normals(unsigned int nb_tris, unsigned int nb_verts, const LVecBase3f * verts, const unsigned int * d_faces, const unsigned short * w_faces, LVecBase3f * normals, bool flip);
  void physx_compute_bounds(LVecBase3f & min, LVecBase3f & max, unsigned int nb_verts, const LVecBase3f * verts);
  void physx_compute_box_around_capsule(const PhysxCapsule & capsule, PhysxBox & box);
  float physx_compute_box_density(const LVecBase3f & extents, float mass);
  void physx_compute_box_inertia_tensor(LVecBase3f & diag_inertia, float mass, float xlength, float ylength, float zlength);
  float physx_compute_box_mass(const LVecBase3f & extents, float density);
  bool physx_compute_box_planes(const PhysxBox & box, PhysxPlane * planes);
  bool physx_compute_box_points(const PhysxBox & box, LVecBase3f * pts);
  bool physx_compute_box_vertex_normals(const PhysxBox & box, LVecBase3f * pts);
  void physx_compute_box_world_edge_normal(const PhysxBox & box, unsigned int edge_index, LVecBase3f & world_normal);
  void physx_compute_capsule_around_box(const PhysxBox & box, PhysxCapsule & capsule);
  float physx_compute_cone_density(float radius, float length, float mass);
  float physx_compute_cone_mass(float radius, float length, float density);
  float physx_compute_cylinder_density(float radius, float length, float mass);
  float physx_compute_cylinder_mass(float radius, float length, float density);
  float physx_compute_distance_squared(const PhysxRay & ray, const LVecBase3f & point, float * t);
  float physx_compute_ellipsoid_density(const LVecBase3f & extents, float mass);
  float physx_compute_ellipsoid_mass(const LVecBase3f & extents, float density);
  float physx_compute_sphere_density(float radius, float mass);
  void physx_compute_sphere_inertia_tensor(LVecBase3f & diag_inertia, float mass, float radius, bool hollow);
  float physx_compute_sphere_mass(float radius, float density);
  float physx_compute_square_distance(const PhysxSegment & seg, const LVecBase3f & point, float * t);
  unsigned int physx_crc32(const void * buffer, unsigned int nb_bytes);
  void physx_create_box(PhysxBox & box, const PhysxBounds3 & aabb, const LMatrix4f & mat);
  bool physx_diagonalize_inertia_tensor(const LMatrix3f & dense_inertia, LVecBase3f & diagonal_inertia, LMatrix3f & rotation);
  bool physx_fast_compute_sphere(PhysxSphere & sphere, unsigned nb_verts, const LVecBase3f * verts);
  void physx_find_rotation_matrix(const LVecBase3f & x, const LVecBase3f & b, LMatrix3f & m);
  const unsigned int * physx_get_box_edges();
  const int * physx_get_box_edges_axes();
  const LVecBase3f * physx_get_box_local_edge_normals();
  const unsigned int * physx_get_box_quads();
  const unsigned int * physx_get_box_triangles();
  int physx_int_ceil(const float & f);
  int physx_int_chop(const float & f);
  int physx_int_floor(const float & f);
  bool physx_is_box_a_inside_box_b(const PhysxBox & a, const PhysxBox & b);
  void physx_joint_desc__set_global_anchor(PhysxJointDesc & dis, const LVecBase3f & ws_anchor);
  void physx_joint_desc__set_global_axis(PhysxJointDesc & dis, const LVecBase3f & ws_axis);
  void physx_merge_spheres(PhysxSphere & merged, const PhysxSphere & sphere0, const PhysxSphere & sphere1);
  void physx_normal_to_tangents(const LVecBase3f & n, LVecBase3f & t1, LVecBase3f & t2);
  bool physx_ray_aabb_intersect(const LVecBase3f & min, const LVecBase3f & max, const LVecBase3f & origin, const LVecBase3f & dir, LVecBase3f & coord);
  unsigned int physx_ray_aabb_intersect2(const LVecBase3f & min, const LVecBase3f & max, const LVecBase3f & origin, const LVecBase3f & dir, LVecBase3f & coord, float & t);
  bool physx_ray_obb_intersect(const PhysxRay & ray, const LVecBase3f & center, const LVecBase3f & extents, const LMatrix3f & rot);
  bool physx_ray_plane_intersect(const PhysxRay & ray, const PhysxPlane & plane, float & dist, LVecBase3f & point_on_plane);
  bool physx_ray_sphere_intersect(const LVecBase3f & origin, const LVecBase3f & dir, float length, const LVecBase3f & center, float radius, float & hit_time, LVecBase3f & hit_pos);
  bool physx_ray_tri_intersect(const LVecBase3f & orig, const LVecBase3f & dir, const LVecBase3f & vert0, const LVecBase3f & vert1, const LVecBase3f & vert2, float & t, float & u, float & v, bool cull);
  bool physx_segment_aabb_intersect(const LVecBase3f & p0, const LVecBase3f & p1, const LVecBase3f & min, const LVecBase3f & max);
  bool physx_segment_box_intersect(const LVecBase3f & p1, const LVecBase3f & p2, const LVecBase3f & bbox_min, const LVecBase3f & bbox_max, LVecBase3f & intercept);
  bool physx_segment_obb_intersect(const LVecBase3f & p0, const LVecBase3f & p1, const LVecBase3f & center, const LVecBase3f & extents, const LMatrix3f & rot);
  float physx_segment_obb_sqr_dist(const PhysxSegment & segment, const LVecBase3f & c0, const LVecBase3f & e0, const LMatrix3f & r0, float * t, LVecBase3f * p);
  void physx_segment_plane_intersect(const LVecBase3f & v1, const LVecBase3f & v2, const PhysxPlane & plane, float & dist, LVecBase3f & point_on_plane);
  PhysxSepAxis physx_separating_axis(const LVecBase3f & extents0, const LVecBase3f & center0, const LMatrix3f & rotation0, const LVecBase3f & extents1, const LVecBase3f & center1, const LMatrix3f & rotation1, bool full_test);
  void physx_set_fpu_exceptions(bool b);
  void physx_set_fpu_precision24();
  void physx_set_fpu_precision53();
  void physx_set_fpu_precision64();
  void physx_set_fpu_rounding_chop();
  void physx_set_fpu_rounding_down();
  void physx_set_fpu_rounding_near();
  void physx_set_fpu_rounding_up();
  bool physx_sweep_box_box(const PhysxBox & box0, const PhysxBox & box1, const LVecBase3f & dir, float length, LVecBase3f & ip, LVecBase3f & normal, float & min_dist);
  bool physx_sweep_box_capsule(const PhysxBox & box, const PhysxCapsule & lss, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & normal);
  bool physx_sweep_box_sphere(const PhysxBox & box, const PhysxSphere & sphere, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & normal);
  bool physx_sweep_capsule_capsule(const PhysxCapsule & lss0, const PhysxCapsule & lss1, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & ip, LVecBase3f & normal);
  bool physx_sweep_sphere_capsule(const PhysxSphere & sphere, const PhysxCapsule & lss, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & ip, LVecBase3f & normal);
  bool physx_swept_spheres_intersect(const PhysxSphere & sphere0, const LVecBase3f & velocity0, const PhysxSphere & sphere1, const LVecBase3f & velocity1);
  bool physx_tri_box_intersect(const LVecBase3f & vertex0, const LVecBase3f & vertex1, const LVecBase3f & vertex2, const LVecBase3f & center, const LVecBase3f & extents);


public:
  NxUtilLib *nUtilLib;
};

#include "physxUtilLib.I"

#endif // HAVE_PHYSX

#endif // PHYSXUTILLIB_H
