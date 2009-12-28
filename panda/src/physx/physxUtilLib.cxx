// Filename: physxUtilLib.cxx
// Created by:  enn0x (01Nov09)
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

#include "physxUtilLib.h"
#include "physxManager.h"
#include "physxBounds3.h"
#include "physxBox.h"
#include "physxCapsule.h"
#include "physxPlane.h"
#include "physxRay.h"
#include "physxSegment.h"
#include "physxSphere.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_exceptions
//       Access: Published
//  Description: Set FPU precision.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_exceptions(bool b) {

  _ptr->NxSetFPUExceptions(b);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_precision24
//       Access: Published
//  Description: Set FPU precision.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_precision24() {

  _ptr->NxSetFPUPrecision24();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_precision53
//       Access: Published
//  Description: Set FPU precision.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_precision53() {

  _ptr->NxSetFPUPrecision53();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_precision64
//       Access: Published
//  Description: Set FPU precision.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_precision64() {

  _ptr->NxSetFPUPrecision64();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_rounding_chop
//       Access: Published
//  Description: Set FPU precision.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_rounding_chop() {

  _ptr->NxSetFPURoundingChop();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_rounding_down
//       Access: Published
//  Description: Set FPU rounding mode.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_rounding_down() {

  _ptr->NxSetFPURoundingDown();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_rounding_near
//       Access: Published
//  Description: Set FPU rounding mode.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_rounding_near() {

  _ptr->NxSetFPURoundingNear();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::set_fpu_rounding_up
//       Access: Published
//  Description: Set FPU rounding mode.
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
set_fpu_rounding_up() {

  _ptr->NxSetFPURoundingUp();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::int_ceil
//       Access: Published
//  Description: Convert a floating point number to an integer.
////////////////////////////////////////////////////////////////////
int PhysxUtilLib::
int_ceil(const float &f) {

  return _ptr->NxIntCeil(f);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::int_chop
//       Access: Published
//  Description: Convert a floating point number to an integer.
////////////////////////////////////////////////////////////////////
int PhysxUtilLib::
int_chop(const float &f) {

  return _ptr->NxIntChop(f);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::int_floor
//       Access: Published
//  Description: Convert a floating point number to an integer.
////////////////////////////////////////////////////////////////////
int PhysxUtilLib::
int_floor(const float &f) {

  return _ptr->NxIntFloor(f);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::box_contains_point
//       Access: Published
//  Description: Test if an oriented box contains a point.
//
//               \param [in] box
//               \param [in] p
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
box_contains_point(const PhysxBox &box, const LPoint3f &p) {

  return _ptr->NxBoxContainsPoint(box._box, PhysxManager::point3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::create_box
//       Access: Published
//  Description: Create an oriented box from an axis aligned box
//               and a transformation. 
//
//               \param [in] aabb
//               \param [in] mat
////////////////////////////////////////////////////////////////////
PhysxBox PhysxUtilLib::
create_box(const PhysxBounds3 &aabb, const LMatrix4f &mat) {

  PhysxBox box;
  _ptr->NxCreateBox(box._box, aabb._bounds, PhysxManager::mat4_to_nxMat34(mat));
  return box;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_box_world_edge_normal
//       Access: Published
//  Description: Compute and edge normals for an oriented box. This
//               is an averaged normal, from the two faces sharing
//               the edge. The edge index should be from 0 to 11 
//               (i.e. a box has 12 edges).
//
//               \param [in] box
//               \param [in] edge_index
////////////////////////////////////////////////////////////////////
LVector3f PhysxUtilLib::
compute_box_world_edge_normal(const PhysxBox &box, unsigned int edge_index) {

  NxVec3 nNormal;

  nassertr(edge_index < 12, LVector3f::zero());

  _ptr->NxComputeBoxWorldEdgeNormal(box._box, edge_index, nNormal);
  return PhysxManager::nxVec3_to_vec3(nNormal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_capsule_around_box
//       Access: Published
//  Description: Compute a capsule which encloses a box.
//
//               \param [in] box
////////////////////////////////////////////////////////////////////
PhysxCapsule PhysxUtilLib::
compute_capsule_around_box(const PhysxBox &box) {

  PhysxCapsule capsule;
  _ptr->NxComputeCapsuleAroundBox(box._box, capsule._capsule);
  return capsule;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::is_box_a_inside_box_b
//       Access: Published
//  Description: Test if box A is inside another box B. Returns
//               TRUE if box A is inside box B.
//
//               \param [in] a
//               \param [in] b
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
is_box_a_inside_box_b(const PhysxBox &a, const PhysxBox &b) {

  return _ptr->NxIsBoxAInsideBoxB(a._box, b._box);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_box_around_capsule
//       Access: Published
//  Description: Compute a box which encloses a capsule.
//
//               \param [in] capsule
////////////////////////////////////////////////////////////////////
PhysxBox PhysxUtilLib::
compute_box_around_capsule(const PhysxCapsule &capsule) {

  PhysxBox box;
  _ptr->NxComputeBoxAroundCapsule(capsule._capsule, box._box);
  return box;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_distance_squared
//       Access: Published
//  Description: Compute the distance squared from a point to a
//               ray.
//
//               \param [in] ray
//               \param [in] point
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_distance_squared(const PhysxRay &ray, const LPoint3f &point) {

  NxF32 t; // not used
  return _ptr->NxComputeDistanceSquared(ray._ray, PhysxManager::point3_to_nxVec3(point), &t);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_square_distance
//       Access: Published
//  Description: Compute the distance squared from a point to a
//               line segment.
//
//               \param [in] seg
//               \param [in] point
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_square_distance(const PhysxSegment &seg, const LPoint3f &point) {

  NxF32 t; // not used
  return _ptr->NxComputeSquareDistance(seg._segment, PhysxManager::point3_to_nxVec3(point), &t);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::merge_spheres
//       Access: Published
//  Description: Compute an overall bounding sphere for a pair of
//               spheres.
//
//               \param [in] sphere0
//               \param [in] sphere1
////////////////////////////////////////////////////////////////////
PhysxSphere PhysxUtilLib::
merge_spheres(const PhysxSphere &sphere0, const PhysxSphere &sphere1) {

  PhysxSphere merged;
  _ptr->NxMergeSpheres(merged._sphere, sphere0._sphere, sphere1._sphere);
  return merged;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::normal_to_tangents
//       Access: Published
//  Description: Get the tangent vectors associated with a normal.
//
//               \param [in] n
//               \param [out] t1
//               \param [out] t2
////////////////////////////////////////////////////////////////////
void PhysxUtilLib::
normal_to_tangents(const LVector3f &n, LVector3f &t1, LVector3f &t2) {

  NxVec3 nt1;
  NxVec3 nt2;

  _ptr->NxNormalToTangents(PhysxManager::vec3_to_nxVec3(n), nt1, nt2);

  t1.set_x(nt1.x);
  t1.set_y(nt1.y);
  t1.set_z(nt1.z);

  t2.set_x(nt2.x);
  t2.set_y(nt2.y);
  t2.set_z(nt2.z);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::find_rotation_matrix
//       Access: Published
//  Description: Computes a rotation matrix M so that: M * x = b
//               (x and b are unit vectors).
//
//               \param [in] x
//               \param [in] b
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxUtilLib::
find_rotation_matrix(const LVector3f &x, const LVector3f &b) {

  NxMat33 nmat;
  _ptr->NxFindRotationMatrix(PhysxManager::vec3_to_nxVec3(x),
                             PhysxManager::vec3_to_nxVec3(b),
                             nmat);
  return PhysxManager::nxMat33_to_mat3(nmat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_sphere_mass
//       Access: Published
//  Description: Computes mass of a homogeneous sphere according
//               to sphere density. 
//
//               \param [in] radius
//               \param [in] density
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_sphere_mass(float radius, float density) {

  return _ptr->NxComputeSphereMass(radius, density);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_sphere_density
//       Access: Published
//  Description: Computes density of a homogeneous sphere according
//               to sphere mass
//
//               \param [in] radius
//               \param [in] mass
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_sphere_density(float radius, float mass) {

  return _ptr->NxComputeSphereDensity(radius, mass);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_box_mass
//       Access: Published
//  Description: Computes mass of a homogeneous box according to
//               box density.
//
//               \param [in] radius
//               \param [in] density
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_box_mass(const LVector3f &extents, float density) {

  return _ptr->NxComputeBoxMass(PhysxManager::vec3_to_nxVec3(extents), density);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_box_density
//       Access: Published
//  Description: Computes density of a homogeneous box according to
//               box mass.
//
//               \param [in] radius
//               \param [in] mass
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_box_density(const LVector3f &extents, float mass) {

  return _ptr->NxComputeBoxDensity(PhysxManager::vec3_to_nxVec3(extents), mass);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_ellipsoid_mass
//       Access: Published
//  Description: Computes mass of a homogeneous ellipsoid according
//               to ellipsoid density.
//
//               \param [in] radius
//               \param [in] density
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_ellipsoid_mass(const LVector3f &extents, float density ) {

  return _ptr->NxComputeEllipsoidMass(PhysxManager::vec3_to_nxVec3(extents), density);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_ellipsoid_density
//       Access: Published
//  Description: Computes density of a homogeneous ellipsoid
//               according to ellipsoid mass.
//
//               \param [in] radius
//               \param [in] mass
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_ellipsoid_density(const LVector3f &extents, float mass) {

  return _ptr->NxComputeEllipsoidDensity(PhysxManager::vec3_to_nxVec3(extents), mass);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_cylinder_mass
//       Access: Published
//  Description: Computes mass of a homogeneous cylinder according
//               to cylinder density.
//
//               \param [in] radius
//               \param [in] density
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_cylinder_mass(float radius, float length, float density) {

  return _ptr->NxComputeCylinderMass(radius, length, density);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_cylinder_density
//       Access: Published
//  Description: Computes density of a homogeneous cylinder
//               according to cylinder mass.
//
//               \param [in] radius
//               \param [in] mass
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_cylinder_density(float radius, float length, float mass) {

  return _ptr->NxComputeCylinderDensity(radius, length, mass);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_cone_mass
//       Access: Published
//  Description: Computes mass of a homogeneous cone according to
//               cone density.
//
//               \param [in] radius
//               \param [in] density
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_cone_mass(float radius, float length, float density) {

  return _ptr->NxComputeConeMass(radius, length, density);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_cone_density
//       Access: Published
//  Description: Computes density of a homogeneous cone according
//               to cone mass.
//
//               \param [in] radius
//               \param [in] mass
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
compute_cone_density(float radius, float length, float mass) {

  return _ptr->NxComputeConeDensity(radius, length, mass);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_box_inertia_tensor
//       Access: Published
//  Description: Computes diagonalized inertia tensor for a box.
//
//               \param [in] mass
//               \param [in] xlength
//               \param [in] ylength
//               \param [in] zlength
////////////////////////////////////////////////////////////////////
LVector3f PhysxUtilLib::
compute_box_inertia_tensor(float mass, float xlength, float ylength, float zlength) {

  NxVec3 tensor;
  _ptr->NxComputeBoxInertiaTensor(tensor, mass, xlength, ylength, zlength);
  return PhysxManager::nxVec3_to_vec3(tensor);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::compute_sphere_inertia_tensor
//       Access: Published
//  Description: Computes diagonalized inertia tensor for a sphere.
//
//               \param [in] mass
//               \param [in] radius
//               \param [in] hollow
////////////////////////////////////////////////////////////////////
LVector3f PhysxUtilLib::
compute_sphere_inertia_tensor(float mass, float radius, bool hollow) {

  NxVec3 tensor;
  _ptr->NxComputeSphereInertiaTensor(tensor, mass, radius, hollow);
  return PhysxManager::nxVec3_to_vec3(tensor);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::box_box_intersect
//       Access: Published
//  Description: Boolean intersection test between two OBBs. Uses
//               the separating axis theorem. Disabling 'full_test'
//               only performs 6 axis tests out of 15.
//
//               \param [in] extents0
//               \param [in] center0
//               \param [in] rotation0
//               \param [in] extents1
//               \param [in] center1
//               \param [in] rotation1
//               \param [in] full_test
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
box_box_intersect(const LVector3f &extents0, const LPoint3f &center0, const LMatrix3f &rotation0, const LVector3f &extents1, const LPoint3f &center1, const LMatrix3f &rotation1, bool full_test) {

  nassertr_always(!extents0.is_nan(), false);
  nassertr_always(!center0.is_nan(), false);
  nassertr_always(!rotation0.is_nan(), false);
  nassertr_always(!extents1.is_nan(), false);
  nassertr_always(!center1.is_nan(), false);
  nassertr_always(!rotation1.is_nan(), false);

  return _ptr->NxBoxBoxIntersect(
    PhysxManager::vec3_to_nxVec3(extents0),
    PhysxManager::point3_to_nxVec3(center0),
    PhysxManager::mat3_to_nxMat33(rotation0),
    PhysxManager::vec3_to_nxVec3(extents1),
    PhysxManager::point3_to_nxVec3(center1),
    PhysxManager::mat3_to_nxMat33(rotation1), full_test);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::tri_box_intersect
//       Access: Published
//  Description: Boolean intersection test between a triangle and
//               a box.
//
//               \param [in] vertex0
//               \param [in] vertex1
//               \param [in] vertex2
//               \param [in] center
//               \param [in] extents
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
tri_box_intersect(const LPoint3f &vertex0, const LPoint3f &vertex1, const LPoint3f &vertex2, const LPoint3f &center, const LVector3f &extents) {

  nassertr_always(!vertex0.is_nan(), false);
  nassertr_always(!vertex1.is_nan(), false);
  nassertr_always(!vertex2.is_nan(), false);
  nassertr_always(!center.is_nan(), false);
  nassertr_always(!extents.is_nan(), false);

  return _ptr->NxTriBoxIntersect(
    PhysxManager::point3_to_nxVec3(vertex0),
    PhysxManager::point3_to_nxVec3(vertex1),
    PhysxManager::point3_to_nxVec3(vertex2),
    PhysxManager::point3_to_nxVec3(center),
    PhysxManager::point3_to_nxVec3(extents));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::ray_plane_intersect
//       Access: Published
//  Description: Ray-plane intersection test. 
//
//               \param [in] ray
//               \param [in] plane
//               \param [out] point_on_plane
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
ray_plane_intersect(const PhysxRay &ray, const PhysxPlane &plane, LPoint3f &point_on_plane) {

  NxReal dist; // not used
  NxVec3 nPointOnPlane;

  bool result = _ptr->NxRayPlaneIntersect(ray._ray, plane._plane, dist, nPointOnPlane);

  PhysxManager::update_point3_from_nxVec3(point_on_plane, nPointOnPlane);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::ray_sphere_intersect
//       Access: Published
//  Description: Ray-sphere intersection test. Returns true if the
//               ray intersects the sphere, and the impact point if
//               needed.
//
//               \param [in] origin
//               \param [in] dir
//               \param [in] length
//               \param [in] center
//               \param [in] radius
//               \param [out] hit_pos
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
ray_sphere_intersect(const LPoint3f &origin, const LVector3f &dir, float length, const LPoint3f &center, float radius, LPoint3f &hit_pos) {

  nassertr_always(!origin.is_nan(), false);
  nassertr_always(!dir.is_nan(), false);
  nassertr_always(!center.is_nan(), false);

  NxReal nHitTime; // not used
  NxVec3 nPointOnPlane;

  bool result = _ptr->NxRaySphereIntersect(
    PhysxManager::point3_to_nxVec3(origin),
    PhysxManager::vec3_to_nxVec3(dir),
    length,
    PhysxManager::point3_to_nxVec3(center),
    radius,
    nHitTime,
    nPointOnPlane);

  PhysxManager::update_point3_from_nxVec3(hit_pos, nPointOnPlane);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::segment_box_intersect
//       Access: Published
//  Description: Segment-AABB intersection test. Also computes
//               intersection point.
//
//               \param [in] p1
//               \param [in] p2
//               \param [in] bbox_min
//               \param [in] bbox_max
//               \param [out] intercept
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
segment_box_intersect(const LPoint3f &p1, const LPoint3f &p2, const LPoint3f &bbox_min, const LPoint3f &bbox_max, LPoint3f &intercept) {

  nassertr_always(!p1.is_nan(), false);
  nassertr_always(!p2.is_nan(), false);
  nassertr_always(!bbox_min.is_nan(), false);
  nassertr_always(!bbox_max.is_nan(), false);

  NxVec3 nIntercept;

  bool result =_ptr->NxSegmentBoxIntersect(
    PhysxManager::point3_to_nxVec3(p1),
    PhysxManager::point3_to_nxVec3(p2),
    PhysxManager::point3_to_nxVec3(bbox_min),
    PhysxManager::point3_to_nxVec3(bbox_max),
    nIntercept);

  PhysxManager::update_point3_from_nxVec3(intercept, nIntercept);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::ray_aabb_intersect
//       Access: Published
//  Description: Ray-AABB intersection test. Also computes
//               intersection point.
//
//               \param [in] min
//               \param [in] max
//               \param [in] origin
//               \param [in] dir
//               \param [out] coord
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
ray_aabb_intersect(const LPoint3f &min, const LPoint3f &max, const LPoint3f &origin, const LVector3f &dir, LPoint3f &coord) {

  nassertr_always(!min.is_nan(), false);
  nassertr_always(!max.is_nan(), false);
  nassertr_always(!origin.is_nan(), false);
  nassertr_always(!dir.is_nan(), false);

  NxVec3 nCoord;

  bool result = _ptr->NxRayAABBIntersect(
    PhysxManager::point3_to_nxVec3(min),
    PhysxManager::point3_to_nxVec3(max),
    PhysxManager::point3_to_nxVec3(origin),
    PhysxManager::vec3_to_nxVec3(dir),
    nCoord);

  PhysxManager::update_point3_from_nxVec3(coord, nCoord);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::segment_obb_intersect
//       Access: Published
//  Description: Boolean segment-OBB intersection test. Based on
//               separating axis theorem.
//
//               \param [in] p0
//               \param [in] p1
//               \param [in] center
//               \param [in] extents
//               \param [in] rot
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
segment_obb_intersect(const LPoint3f &p0, const LPoint3f &p1, const LPoint3f &center, const LVector3f &extents, const LMatrix3f &rot) {

  nassertr_always(!p0.is_nan(), false);
  nassertr_always(!p1.is_nan(), false);
  nassertr_always(!center.is_nan(), false);
  nassertr_always(!extents.is_nan(), false);
  nassertr_always(!rot.is_nan(), false);

  return _ptr->NxSegmentOBBIntersect(
    PhysxManager::point3_to_nxVec3(p0),
    PhysxManager::point3_to_nxVec3(p1),
    PhysxManager::point3_to_nxVec3(center),
    PhysxManager::vec3_to_nxVec3(extents),
    PhysxManager::mat3_to_nxMat33(rot));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::segment_aabb_intersect
//       Access: Published
//  Description: Boolean segment-AABB intersection test. Based on
//               separating axis theorem.
//
//               \param [in] p0
//               \param [in] p1
//               \param [in] min
//               \param [in] max
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
segment_aabb_intersect(const LPoint3f &p0, const LPoint3f &p1, const LPoint3f &min, const LPoint3f &max) {

  nassertr_always(!p0.is_nan(), false);
  nassertr_always(!p1.is_nan(), false);
  nassertr_always(!min.is_nan(), false);
  nassertr_always(!max.is_nan(), false);

  return _ptr->NxSegmentAABBIntersect(
    PhysxManager::point3_to_nxVec3(p0),
    PhysxManager::point3_to_nxVec3(p1),
    PhysxManager::point3_to_nxVec3(min),
    PhysxManager::point3_to_nxVec3(max));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::ray_obb_intersect
//       Access: Published
//  Description: Boolean ray-OBB intersection test. Based on
//               separating axis theorem.
//
//               \param [in] ray
//               \param [in] center
//               \param [in] extents
//               \param [in] rot
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
ray_obb_intersect(const PhysxRay &ray, const LPoint3f &center, const LVector3f &extents, const LMatrix3f &rot) {

  nassertr_always(!center.is_nan(), false);
  nassertr_always(!extents.is_nan(), false);
  nassertr_always(!rot.is_nan(), false);

  return _ptr->NxRayOBBIntersect(
    ray._ray,
    PhysxManager::point3_to_nxVec3(center),
    PhysxManager::point3_to_nxVec3(extents),
    PhysxManager::mat3_to_nxMat33(rot));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::ray_capsule_intersect
//       Access: Published
//  Description: Ray-capsule intersection test. Returns number of
//               intersection points (0,1 or 2) along the ray.
//
//               \param [in] origin
//               \param [in] dir
//               \param [in] capsule
////////////////////////////////////////////////////////////////////
unsigned int PhysxUtilLib::
ray_capsule_intersect(const LPoint3f &origin, const LVector3f &dir, const PhysxCapsule &capsule) {

  nassertr_always(!origin.is_nan(), -1);
  nassertr_always(!dir.is_nan(), -1);

  NxReal t[2] = { 0.0f, 0.0f }; // not used

  return _ptr->NxRayCapsuleIntersect(
    PhysxManager::point3_to_nxVec3(origin),
    PhysxManager::vec3_to_nxVec3(dir),
    capsule._capsule, t);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::swept_spheres_intersect
//       Access: Published
//  Description: Sphere-sphere sweep test. Returns true if spheres
//               intersect during their linear motion along
//               provided velocity vectors.
//
//               \param [in] sphere0
//               \param [in] velocity0
//               \param [in] sphere1
//               \param [in] velocity1
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
swept_spheres_intersect(const PhysxSphere &sphere0, const LVector3f &velocity0, const PhysxSphere &sphere1, const LVector3f &velocity1) {

  nassertr_always(!velocity0.is_nan(), false);
  nassertr_always(!velocity1.is_nan(), false);

  return _ptr->NxSweptSpheresIntersect(
    sphere0._sphere,
    PhysxManager::vec3_to_nxVec3(velocity0),
    sphere1._sphere,
    PhysxManager::vec3_to_nxVec3(velocity1));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::ray_tri_intersect
//       Access: Published
//  Description: Ray-triangle intersection test. Returns impact 
//               distance (t) as well as barycentric coordinates
//               (u,v) of impact point. The test performs back face
//               culling or not according to 'cull'.
//
//               \param [in] orig
//               \param [in] dir
//               \param [in] vert0
//               \param [in] vert1
//               \param [in] vert2
//               \param [out] hit, with coordinates (t,u,v)
//               \param [in] cull
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
ray_tri_intersect(const LPoint3f &orig, const LVector3f &dir, const LPoint3f &vert0, const LPoint3f &vert1, const LPoint3f &vert2, LVector3f &hit, bool cull) {

  nassertr_always(!orig.is_nan(), false);
  nassertr_always(!dir.is_nan(), false);
  nassertr_always(!vert0.is_nan(), false);
  nassertr_always(!vert1.is_nan(), false);
  nassertr_always(!vert2.is_nan(), false);

  NxReal t, u, v;

  bool result = _ptr->NxRayTriIntersect(
    PhysxManager::point3_to_nxVec3(orig),
    PhysxManager::vec3_to_nxVec3(dir),
    PhysxManager::point3_to_nxVec3(vert0),
    PhysxManager::point3_to_nxVec3(vert1),
    PhysxManager::point3_to_nxVec3(vert2),
    t, u, v, cull);

  hit.set_x(t);
  hit.set_y(u);
  hit.set_z(v);

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::sweep_box_capsule
//       Access: Published
//  Description: Box-vs-capsule sweep test. Sweeps a box against a
//               capsule, returns true if box hit the capsule. Also
//               returns contact information.
//
//               \param [in] box Box
//               \param [in] lss Capsule
//               \param [in] dir Unit-length sweep direction
//               \param [in] length Length of sweep
//               \param [out] normal Normal at impact point
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
sweep_box_capsule(const PhysxBox &box, const PhysxCapsule &lss, const LVector3f &dir, float length, LVector3f &normal) {

  nassertr_always(!dir.is_nan(), false);

  NxReal min_dist; // not used
  NxVec3 nNormal;

  bool result = _ptr->NxSweepBoxCapsule(
    box._box, lss._capsule,
    PhysxManager::vec3_to_nxVec3(dir),
    length, min_dist, nNormal);

  PhysxManager::update_vec3_from_nxVec3(normal, nNormal);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::sweep_box_sphere
//       Access: Published
//  Description: Box-vs-sphere sweep test. Sweeps a box against a
//               sphere, returns true if box hit the sphere. Also
//               returns contact information.
//
//               \param [in] box Box
//               \param [in] sphere Sphere
//               \param [in] dir Unit-length sweep direction
//               \param [in] length Length of sweep
//               \param [out] normal Normal at impact point
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
sweep_box_sphere(const PhysxBox &box, const PhysxSphere &sphere, const LVector3f &dir, float length, LVector3f &normal) {

  nassertr_always(!dir.is_nan(), false);

  NxReal min_dist; // not used
  NxVec3 nNormal;

  bool result = _ptr->NxSweepBoxSphere(
    box._box, sphere._sphere,
    PhysxManager::vec3_to_nxVec3(dir),
    length, min_dist, nNormal);

  PhysxManager::update_vec3_from_nxVec3(normal, nNormal);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::sweep_capsule_capsule
//       Access: Published
//  Description: Capsule-vs-capsule sweep test. Sweeps a capsule
//               against a capsule, returns true if capsule hit the
//               other capsule. Also returns contact information.
//
//               \param [in] lss0
//               \param [in] lss1
//               \param [in] dir Unit-length sweep direction
//               \param [in] length Length of sweep
//               \param [out] ip Impact point
//               \param [out] normal Normal at impact point
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
sweep_capsule_capsule(const PhysxCapsule &lss0, const PhysxCapsule &lss1, const LVector3f &dir, float length, LPoint3f &ip, LVector3f &normal) {

  nassertr_always(!dir.is_nan(), false);

  NxReal min_dist; // not used
  NxVec3 nIp;
  NxVec3 nNormal;

  bool result = _ptr->NxSweepCapsuleCapsule(
    lss0._capsule, lss1._capsule,
    PhysxManager::vec3_to_nxVec3(dir),
    length, min_dist, nIp, nNormal);

  PhysxManager::update_point3_from_nxVec3(ip, nIp);
  PhysxManager::update_vec3_from_nxVec3(normal, nNormal);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::sweep_sphere_capsule
//       Access: Published
//  Description: Sphere-vs-capsule sweep test. Sweeps a sphere
//               against a capsule, returns true if sphere hit the
//               capsule. Also returns contact information.
//
//               \param [in] sphere
//               \param [in] lss
//               \param [in] dir Unit-length sweep direction
//               \param [in] length Length of sweep
//               \param [out] ip Impact point
//               \param [out] normal Normal at impact point
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
sweep_sphere_capsule(const PhysxSphere &sphere, const PhysxCapsule &lss, const LVector3f &dir, float length, LPoint3f &ip, LVector3f &normal) {

  nassertr_always(!dir.is_nan(), false);

  NxReal min_dist; // not used
  NxVec3 nIp;
  NxVec3 nNormal;

  bool result = _ptr->NxSweepSphereCapsule(
    sphere._sphere, lss._capsule,
    PhysxManager::vec3_to_nxVec3(dir),
    length, min_dist, nIp, nNormal);

  PhysxManager::update_point3_from_nxVec3(ip, nIp);
  PhysxManager::update_vec3_from_nxVec3(normal, nNormal);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::sweep_box_box
//       Access: Published
//  Description: Box-vs-box sweep test. Sweeps a box against a box,
//               returns true if box hit the other box. Also returns
//               contact information.
//
//               \param [in] box0
//               \param [in] box1
//               \param [in] dir Unit-length sweep direction
//               \param [in] length Length of sweep
//               \param [out] ip Impact point
//               \param [out] normal Normal at impact point
////////////////////////////////////////////////////////////////////
bool PhysxUtilLib::
sweep_box_box(const PhysxBox &box0, const PhysxBox &box1, const LVector3f &dir, float length, LPoint3f &ip, LVector3f &normal) {

  nassertr_always(!dir.is_nan(), false);

  NxReal min_dist; // not used
  NxVec3 nIp;
  NxVec3 nNormal;

  bool result = _ptr->NxSweepBoxBox(
    box0._box, box1._box,
    PhysxManager::vec3_to_nxVec3(dir),
    length, nIp, nNormal, min_dist);

  PhysxManager::update_point3_from_nxVec3(ip, nIp);
  PhysxManager::update_vec3_from_nxVec3(normal, nNormal);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::point_obb_sqr_dist
//       Access: Published
//  Description: Point-vs-OBB distance computation. Returns distance
//               between a point and an OBB.
//
//               \param [in] point The point
//               \param [in] center OBB center
//               \param [in] extents OBB extents
//               \param [in] rot OBB rotation
//               \param [out] params Closest point on the box, in box space
////////////////////////////////////////////////////////////////////
float PhysxUtilLib::
point_obb_sqr_dist(const LPoint3f &point, const LPoint3f &center, const LVector3f &extents, const LMatrix3f &rot, LPoint3f &params) {

  nassertr_always(!point.is_nan(), 0.0f);
  nassertr_always(!center.is_nan(), 0.0f);
  nassertr_always(!extents.is_nan(), 0.0f);
  nassertr_always(!rot.is_nan(), 0.0f);

  NxVec3 nParams;

  float result = _ptr->NxPointOBBSqrDist(
    PhysxManager::point3_to_nxVec3(point),
    PhysxManager::point3_to_nxVec3(center),
    PhysxManager::vec3_to_nxVec3(extents),
    PhysxManager::mat3_to_nxMat33(rot),
    &nParams);

  PhysxManager::update_point3_from_nxVec3(params, nParams);
  return result;
}

