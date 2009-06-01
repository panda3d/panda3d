// Filename: physxUtilLib.cxx
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

#ifdef HAVE_PHYSX

#include "physxUtilLib.h"

#include "luse.h"
#include "physxBounds3.h"
#include "physxBox.h"
#include "physxCapsule.h"
#include "physxJointDesc.h"
#include "physxPlane.h"
#include "physxRay.h"
#include "physxSegment.h"
#include "physxSphere.h"


////////////////////////////////////////////////////////////////////
//     Function : physx_box_box_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_box_box_intersect(const LVecBase3f & extents0, const LVecBase3f & center0, const LMatrix3f & rotation0, const LVecBase3f & extents1, const LVecBase3f & center1, const LMatrix3f & rotation1, bool full_test) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxBoxBoxIntersect(PhysxManager::lVecBase3_to_nxVec3(extents0), PhysxManager::lVecBase3_to_nxVec3(center0), PhysxManager::lMatrix3_to_nxMat33(rotation0), PhysxManager::lVecBase3_to_nxVec3(extents1), PhysxManager::lVecBase3_to_nxVec3(center1), PhysxManager::lMatrix3_to_nxMat33(rotation1), full_test);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_box_contains_point
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_box_contains_point(const PhysxBox & box, const LVecBase3f & p) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxBoxContainsPoint(*(box.nBox), PhysxManager::lVecBase3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_box_vertex_to_quad
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const unsigned int * PhysxUtilLib::
physx_box_vertex_to_quad(unsigned int vertex_index) {
  nassertr(nUtilLib != NULL, NULL);

  return nUtilLib->NxBoxVertexToQuad(vertex_index);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_build_smooth_normals
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_build_smooth_normals(unsigned int nb_tris, unsigned int nb_verts, const LVecBase3f * verts, const unsigned int * d_faces, const unsigned short * w_faces, LVecBase3f * normals, bool flip) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxBuildSmoothNormals(nb_tris, nb_verts, &PhysxManager::lVecBase3_to_nxVec3(*verts), d_faces, w_faces, &PhysxManager::lVecBase3_to_nxVec3(*normals), flip);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_bounds
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_compute_bounds(LVecBase3f & min, LVecBase3f & max, unsigned int nb_verts, const LVecBase3f * verts) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxComputeBounds(PhysxManager::lVecBase3_to_nxVec3(min), PhysxManager::lVecBase3_to_nxVec3(max), nb_verts, &PhysxManager::lVecBase3_to_nxVec3(*verts));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_around_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_compute_box_around_capsule(const PhysxCapsule & capsule, PhysxBox & box) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxComputeBoxAroundCapsule(*(capsule.nCapsule), *(box.nBox));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_density
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_box_density(const LVecBase3f & extents, float mass) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeBoxDensity(PhysxManager::lVecBase3_to_nxVec3(extents), mass);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_inertia_tensor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_compute_box_inertia_tensor(LVecBase3f & diag_inertia, float mass, float xlength, float ylength, float zlength) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxComputeBoxInertiaTensor(PhysxManager::lVecBase3_to_nxVec3(diag_inertia), mass, xlength, ylength, zlength);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_box_mass(const LVecBase3f & extents, float density) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeBoxMass(PhysxManager::lVecBase3_to_nxVec3(extents), density);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_planes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_compute_box_planes(const PhysxBox & box, PhysxPlane * planes) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxComputeBoxPlanes(*(box.nBox), planes->nPlane);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_points
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_compute_box_points(const PhysxBox & box, LVecBase3f * pts) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxComputeBoxPoints(*(box.nBox), &PhysxManager::lVecBase3_to_nxVec3(*pts));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_vertex_normals
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_compute_box_vertex_normals(const PhysxBox & box, LVecBase3f * pts) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxComputeBoxVertexNormals(*(box.nBox), &PhysxManager::lVecBase3_to_nxVec3(*pts));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_box_world_edge_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_compute_box_world_edge_normal(const PhysxBox & box, unsigned int edge_index, LVecBase3f & world_normal) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxComputeBoxWorldEdgeNormal(*(box.nBox), edge_index, PhysxManager::lVecBase3_to_nxVec3(world_normal));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_capsule_around_box
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_compute_capsule_around_box(const PhysxBox & box, PhysxCapsule & capsule) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxComputeCapsuleAroundBox(*(box.nBox), *(capsule.nCapsule));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_cone_density
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_cone_density(float radius, float length, float mass) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeConeDensity(radius, length, mass);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_cone_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_cone_mass(float radius, float length, float density) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeConeMass(radius, length, density);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_cylinder_density
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_cylinder_density(float radius, float length, float mass) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeCylinderDensity(radius, length, mass);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_cylinder_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_cylinder_mass(float radius, float length, float density) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeCylinderMass(radius, length, density);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_distance_squared
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_distance_squared(const PhysxRay & ray, const LVecBase3f & point, float * t) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeDistanceSquared(*(ray.nRay), PhysxManager::lVecBase3_to_nxVec3(point), t);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_ellipsoid_density
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_ellipsoid_density(const LVecBase3f & extents, float mass) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeEllipsoidDensity(PhysxManager::lVecBase3_to_nxVec3(extents), mass);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_ellipsoid_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_ellipsoid_mass(const LVecBase3f & extents, float density) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeEllipsoidMass(PhysxManager::lVecBase3_to_nxVec3(extents), density);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_sphere_density
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_sphere_density(float radius, float mass) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeSphereDensity(radius, mass);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_sphere_inertia_tensor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_compute_sphere_inertia_tensor(LVecBase3f & diag_inertia, float mass, float radius, bool hollow) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxComputeSphereInertiaTensor(PhysxManager::lVecBase3_to_nxVec3(diag_inertia), mass, radius, hollow);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_sphere_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_sphere_mass(float radius, float density) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeSphereMass(radius, density);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_compute_square_distance
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_compute_square_distance(const PhysxSegment & seg, const LVecBase3f & point, float * t) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxComputeSquareDistance(*(seg.nSegment), PhysxManager::lVecBase3_to_nxVec3(point), t);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_crc32
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxUtilLib::
physx_crc32(const void * buffer, unsigned int nb_bytes) {
  nassertr(nUtilLib != NULL, -1);

  return nUtilLib->NxCrc32(buffer, nb_bytes);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_create_box
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_create_box(PhysxBox & box, const PhysxBounds3 & aabb, const LMatrix4f & mat) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxCreateBox(*(box.nBox), *(aabb.nBounds3), PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_diagonalize_inertia_tensor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_diagonalize_inertia_tensor(const LMatrix3f & dense_inertia, LVecBase3f & diagonal_inertia, LMatrix3f & rotation) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxDiagonalizeInertiaTensor(PhysxManager::lMatrix3_to_nxMat33(dense_inertia), PhysxManager::lVecBase3_to_nxVec3(diagonal_inertia), PhysxManager::lMatrix3_to_nxMat33(rotation));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_fast_compute_sphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_fast_compute_sphere(PhysxSphere & sphere, unsigned nb_verts, const LVecBase3f * verts) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxFastComputeSphere(*(sphere.nSphere), nb_verts, &PhysxManager::lVecBase3_to_nxVec3(*verts));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_find_rotation_matrix
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_find_rotation_matrix(const LVecBase3f & x, const LVecBase3f & b, LMatrix3f & m) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxFindRotationMatrix(PhysxManager::lVecBase3_to_nxVec3(x), PhysxManager::lVecBase3_to_nxVec3(b), PhysxManager::lMatrix3_to_nxMat33(m));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_get_box_edges
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const unsigned int * PhysxUtilLib::
physx_get_box_edges() {
  nassertr(nUtilLib != NULL, NULL);

  return nUtilLib->NxGetBoxEdges();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_get_box_edges_axes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const int * PhysxUtilLib::
physx_get_box_edges_axes() {
  nassertr(nUtilLib != NULL, NULL);

  return nUtilLib->NxGetBoxEdgesAxes();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_get_box_local_edge_normals
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const LVecBase3f * PhysxUtilLib::
physx_get_box_local_edge_normals() {
  nassertr(nUtilLib != NULL, NULL);

  return &PhysxManager::nxVec3_to_lVecBase3(*nUtilLib->NxGetBoxLocalEdgeNormals());
}

////////////////////////////////////////////////////////////////////
//     Function : physx_get_box_quads
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const unsigned int * PhysxUtilLib::
physx_get_box_quads() {
  nassertr(nUtilLib != NULL, NULL);

  return nUtilLib->NxGetBoxQuads();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_get_box_triangles
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const unsigned int * PhysxUtilLib::
physx_get_box_triangles() {
  nassertr(nUtilLib != NULL, NULL);

  return nUtilLib->NxGetBoxTriangles();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_int_ceil
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
int PhysxUtilLib::
physx_int_ceil(const float & f) {
  return nUtilLib->NxIntCeil(f);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_int_chop
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
int PhysxUtilLib::
physx_int_chop(const float & f) {
  return nUtilLib->NxIntChop(f);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_int_floor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
int PhysxUtilLib::
physx_int_floor(const float & f) {
  return nUtilLib->NxIntFloor(f);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_is_box_a_inside_box_b
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_is_box_a_inside_box_b(const PhysxBox & a, const PhysxBox & b) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxIsBoxAInsideBoxB(*(a.nBox), *(b.nBox));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_joint_desc__set_global_anchor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_joint_desc__set_global_anchor(PhysxJointDesc & dis, const LVecBase3f & ws_anchor) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxJointDesc_SetGlobalAnchor(*(dis.nJointDesc), PhysxManager::lVecBase3_to_nxVec3(ws_anchor));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_joint_desc__set_global_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_joint_desc__set_global_axis(PhysxJointDesc & dis, const LVecBase3f & ws_axis) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxJointDesc_SetGlobalAxis(*(dis.nJointDesc), PhysxManager::lVecBase3_to_nxVec3(ws_axis));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_merge_spheres
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_merge_spheres(PhysxSphere & merged, const PhysxSphere & sphere0, const PhysxSphere & sphere1) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxMergeSpheres(*(merged.nSphere), *(sphere0.nSphere), *(sphere1.nSphere));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_normal_to_tangents
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_normal_to_tangents(const LVecBase3f & n, LVecBase3f & t1, LVecBase3f & t2) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxNormalToTangents(PhysxManager::lVecBase3_to_nxVec3(n), PhysxManager::lVecBase3_to_nxVec3(t1), PhysxManager::lVecBase3_to_nxVec3(t2));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_ray_aabb_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_ray_aabb_intersect(const LVecBase3f & min, const LVecBase3f & max, const LVecBase3f & origin, const LVecBase3f & dir, LVecBase3f & coord) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxRayAABBIntersect(PhysxManager::lVecBase3_to_nxVec3(min), PhysxManager::lVecBase3_to_nxVec3(max), PhysxManager::lVecBase3_to_nxVec3(origin), PhysxManager::lVecBase3_to_nxVec3(dir), PhysxManager::lVecBase3_to_nxVec3(coord));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_ray_aabb_intersect2
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxUtilLib::
physx_ray_aabb_intersect2(const LVecBase3f & min, const LVecBase3f & max, const LVecBase3f & origin, const LVecBase3f & dir, LVecBase3f & coord, float & t) {
  nassertr(nUtilLib != NULL, -1);

  return nUtilLib->NxRayAABBIntersect2(PhysxManager::lVecBase3_to_nxVec3(min), PhysxManager::lVecBase3_to_nxVec3(max), PhysxManager::lVecBase3_to_nxVec3(origin), PhysxManager::lVecBase3_to_nxVec3(dir), PhysxManager::lVecBase3_to_nxVec3(coord), t);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_ray_obb_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_ray_obb_intersect(const PhysxRay & ray, const LVecBase3f & center, const LVecBase3f & extents, const LMatrix3f & rot) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxRayOBBIntersect(*(ray.nRay), PhysxManager::lVecBase3_to_nxVec3(center), PhysxManager::lVecBase3_to_nxVec3(extents), PhysxManager::lMatrix3_to_nxMat33(rot));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_ray_plane_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_ray_plane_intersect(const PhysxRay & ray, const PhysxPlane & plane, float & dist, LVecBase3f & point_on_plane) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxRayPlaneIntersect(*(ray.nRay), *(plane.nPlane), dist, PhysxManager::lVecBase3_to_nxVec3(point_on_plane));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_ray_sphere_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_ray_sphere_intersect(const LVecBase3f & origin, const LVecBase3f & dir, float length, const LVecBase3f & center, float radius, float & hit_time, LVecBase3f & hit_pos) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxRaySphereIntersect(PhysxManager::lVecBase3_to_nxVec3(origin), PhysxManager::lVecBase3_to_nxVec3(dir), length, PhysxManager::lVecBase3_to_nxVec3(center), radius, hit_time, PhysxManager::lVecBase3_to_nxVec3(hit_pos));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_ray_tri_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_ray_tri_intersect(const LVecBase3f & orig, const LVecBase3f & dir, const LVecBase3f & vert0, const LVecBase3f & vert1, const LVecBase3f & vert2, float & t, float & u, float & v, bool cull) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxRayTriIntersect(PhysxManager::lVecBase3_to_nxVec3(orig), PhysxManager::lVecBase3_to_nxVec3(dir), PhysxManager::lVecBase3_to_nxVec3(vert0), PhysxManager::lVecBase3_to_nxVec3(vert1), PhysxManager::lVecBase3_to_nxVec3(vert2), t, u, v, cull);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_segment_aabb_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_segment_aabb_intersect(const LVecBase3f & p0, const LVecBase3f & p1, const LVecBase3f & min, const LVecBase3f & max) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSegmentAABBIntersect(PhysxManager::lVecBase3_to_nxVec3(p0), PhysxManager::lVecBase3_to_nxVec3(p1), PhysxManager::lVecBase3_to_nxVec3(min), PhysxManager::lVecBase3_to_nxVec3(max));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_segment_box_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_segment_box_intersect(const LVecBase3f & p1, const LVecBase3f & p2, const LVecBase3f & bbox_min, const LVecBase3f & bbox_max, LVecBase3f & intercept) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSegmentBoxIntersect(PhysxManager::lVecBase3_to_nxVec3(p1), PhysxManager::lVecBase3_to_nxVec3(p2), PhysxManager::lVecBase3_to_nxVec3(bbox_min), PhysxManager::lVecBase3_to_nxVec3(bbox_max), PhysxManager::lVecBase3_to_nxVec3(intercept));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_segment_obb_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_segment_obb_intersect(const LVecBase3f & p0, const LVecBase3f & p1, const LVecBase3f & center, const LVecBase3f & extents, const LMatrix3f & rot) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSegmentOBBIntersect(PhysxManager::lVecBase3_to_nxVec3(p0), PhysxManager::lVecBase3_to_nxVec3(p1), PhysxManager::lVecBase3_to_nxVec3(center), PhysxManager::lVecBase3_to_nxVec3(extents), PhysxManager::lMatrix3_to_nxMat33(rot));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_segment_obb_sqr_dist
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
physx_segment_obb_sqr_dist(const PhysxSegment & segment, const LVecBase3f & c0, const LVecBase3f & e0, const LMatrix3f & r0, float * t, LVecBase3f * p) {
  nassertr(nUtilLib != NULL, -1.0f);

  return nUtilLib->NxSegmentOBBSqrDist(*(segment.nSegment), PhysxManager::lVecBase3_to_nxVec3(c0), PhysxManager::lVecBase3_to_nxVec3(e0), PhysxManager::lMatrix3_to_nxMat33(r0), t, &PhysxManager::lVecBase3_to_nxVec3(*p));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_segment_plane_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_segment_plane_intersect(const LVecBase3f & v1, const LVecBase3f & v2, const PhysxPlane & plane, float & dist, LVecBase3f & point_on_plane) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSegmentPlaneIntersect(PhysxManager::lVecBase3_to_nxVec3(v1), PhysxManager::lVecBase3_to_nxVec3(v2), *(plane.nPlane), dist, PhysxManager::lVecBase3_to_nxVec3(point_on_plane));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_separating_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSepAxis PhysxUtilLib::
physx_separating_axis(const LVecBase3f & extents0, const LVecBase3f & center0, const LMatrix3f & rotation0, const LVecBase3f & extents1, const LVecBase3f & center1, const LMatrix3f & rotation1, bool full_test) {
  return (PhysxSepAxis)nUtilLib->NxSeparatingAxis(PhysxManager::lVecBase3_to_nxVec3(extents0), PhysxManager::lVecBase3_to_nxVec3(center0), PhysxManager::lMatrix3_to_nxMat33(rotation0), PhysxManager::lVecBase3_to_nxVec3(extents1), PhysxManager::lVecBase3_to_nxVec3(center1), PhysxManager::lMatrix3_to_nxMat33(rotation1), full_test);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_exceptions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_exceptions(bool b) {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPUExceptions(b);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_precision24
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_precision24() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPUPrecision24();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_precision53
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_precision53() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPUPrecision53();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_precision64
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_precision64() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPUPrecision64();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_rounding_chop
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_rounding_chop() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPURoundingChop();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_rounding_down
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_rounding_down() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPURoundingDown();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_rounding_near
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_rounding_near() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPURoundingNear();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_set_fpu_rounding_up
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
physx_set_fpu_rounding_up() {
  nassertv(nUtilLib != NULL);

  nUtilLib->NxSetFPURoundingUp();
}

////////////////////////////////////////////////////////////////////
//     Function : physx_sweep_box_box
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_sweep_box_box(const PhysxBox & box0, const PhysxBox & box1, const LVecBase3f & dir, float length, LVecBase3f & ip, LVecBase3f & normal, float & min_dist) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSweepBoxBox(*(box0.nBox), *(box1.nBox), PhysxManager::lVecBase3_to_nxVec3(dir), length, PhysxManager::lVecBase3_to_nxVec3(ip), PhysxManager::lVecBase3_to_nxVec3(normal), min_dist);
}

////////////////////////////////////////////////////////////////////
//     Function : physx_sweep_box_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_sweep_box_capsule(const PhysxBox & box, const PhysxCapsule & lss, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & normal) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSweepBoxCapsule(*(box.nBox), *(lss.nCapsule), PhysxManager::lVecBase3_to_nxVec3(dir), length, min_dist, PhysxManager::lVecBase3_to_nxVec3(normal));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_sweep_box_sphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_sweep_box_sphere(const PhysxBox & box, const PhysxSphere & sphere, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & normal) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSweepBoxSphere(*(box.nBox), *(sphere.nSphere), PhysxManager::lVecBase3_to_nxVec3(dir), length, min_dist, PhysxManager::lVecBase3_to_nxVec3(normal));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_sweep_capsule_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_sweep_capsule_capsule(const PhysxCapsule & lss0, const PhysxCapsule & lss1, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & ip, LVecBase3f & normal) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSweepCapsuleCapsule(*(lss0.nCapsule), *(lss1.nCapsule), PhysxManager::lVecBase3_to_nxVec3(dir), length, min_dist, PhysxManager::lVecBase3_to_nxVec3(ip), PhysxManager::lVecBase3_to_nxVec3(normal));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_sweep_sphere_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_sweep_sphere_capsule(const PhysxSphere & sphere, const PhysxCapsule & lss, const LVecBase3f & dir, float length, float & min_dist, LVecBase3f & ip, LVecBase3f & normal) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSweepSphereCapsule(*(sphere.nSphere), *(lss.nCapsule), PhysxManager::lVecBase3_to_nxVec3(dir), length, min_dist, PhysxManager::lVecBase3_to_nxVec3(ip), PhysxManager::lVecBase3_to_nxVec3(normal));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_swept_spheres_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_swept_spheres_intersect(const PhysxSphere & sphere0, const LVecBase3f & velocity0, const PhysxSphere & sphere1, const LVecBase3f & velocity1) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxSweptSpheresIntersect(*(sphere0.nSphere), PhysxManager::lVecBase3_to_nxVec3(velocity0), *(sphere1.nSphere), PhysxManager::lVecBase3_to_nxVec3(velocity1));
}

////////////////////////////////////////////////////////////////////
//     Function : physx_tri_box_intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
physx_tri_box_intersect(const LVecBase3f & vertex0, const LVecBase3f & vertex1, const LVecBase3f & vertex2, const LVecBase3f & center, const LVecBase3f & extents) {
  nassertr(nUtilLib != NULL, false);

  return nUtilLib->NxTriBoxIntersect(PhysxManager::lVecBase3_to_nxVec3(vertex0), PhysxManager::lVecBase3_to_nxVec3(vertex1), PhysxManager::lVecBase3_to_nxVec3(vertex2), PhysxManager::lVecBase3_to_nxVec3(center), PhysxManager::lVecBase3_to_nxVec3(extents));
}

#endif // HAVE_PHYSX

