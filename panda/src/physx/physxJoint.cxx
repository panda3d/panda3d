/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJoint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxJoint.h"
#include "physxManager.h"
#include "physxActor.h"
#include "physxScene.h"
#include "physxCylindricalJoint.h"
#include "physxDistanceJoint.h"
#include "physxFixedJoint.h"
#include "physxPointInPlaneJoint.h"
#include "physxPointOnLineJoint.h"
#include "physxPrismaticJoint.h"
#include "physxPulleyJoint.h"
#include "physxRevoluteJoint.h"
#include "physxSphericalJoint.h"
#include "physxD6Joint.h"

TypeHandle PhysxJoint::_type_handle;

/**
 *
 */
void PhysxJoint::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  ptr()->getScene().releaseJoint(*ptr());
}

/**
 *
 */
PhysxJoint *PhysxJoint::
factory(NxJointType shapeType) {

  switch (shapeType) {

  case NX_JOINT_PRISMATIC:
    return new PhysxPrismaticJoint();

  case NX_JOINT_REVOLUTE:
    return new PhysxRevoluteJoint();

  case NX_JOINT_CYLINDRICAL:
    return new PhysxCylindricalJoint();

  case NX_JOINT_SPHERICAL:
    return new PhysxSphericalJoint();

  case NX_JOINT_POINT_ON_LINE:
    return new PhysxPointOnLineJoint();

  case NX_JOINT_POINT_IN_PLANE:
    return new PhysxPointInPlaneJoint();

  case NX_JOINT_DISTANCE:
    return new PhysxDistanceJoint();

  case NX_JOINT_PULLEY:
    return new PhysxPulleyJoint();

  case NX_JOINT_FIXED:
    return new PhysxFixedJoint();

  case NX_JOINT_D6:
    return new PhysxD6Joint();
  }

  physx_cat.error() << "Unknown joint type.\n";
  return nullptr;
}

/**
 * Sets a name string for this object.  The name can be retrieved again with
 * get_name(). This is for debugging and is not used by the physics engine.
 */
void PhysxJoint::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  ptr()->setName(_name.c_str());
}

/**
 * Returns the name string.
 */
const char *PhysxJoint::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return ptr()->getName();
}

/**
 * Retrieves the actor which this joint is associated with.
 */
PhysxActor *PhysxJoint::
get_actor(unsigned int idx) const {

  nassertr_always(idx < 2, nullptr);
  nassertr(_error_type == ET_ok, nullptr);

  NxActor *actorPtr[2];
  ptr()->getActors(&actorPtr[0], &actorPtr[1]);
  return (PhysxActor *)(actorPtr[idx]->userData);
}

/**
 * Retrieves the scene which this joint is associated with.
 */
PhysxScene *PhysxJoint::
get_scene() const {

  nassertr(_error_type == ET_ok, nullptr);
  return (PhysxScene *)(ptr()->getScene().userData);
}

/**
 * Sets the point where the two actors are attached, specified in global
 * coordinates.
 */
void PhysxJoint::
set_global_anchor(const LPoint3f &anchor) {

  nassertv(_error_type == ET_ok);
  ptr()->setGlobalAnchor(PhysxManager::point3_to_nxVec3(anchor));
}

/**
 * Retrieves the joint anchor.
 */
LPoint3f PhysxJoint::
get_global_anchor() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());
  return PhysxManager::nxVec3_to_point3(ptr()->getGlobalAnchor());
}

/**
 * Sets the direction of the joint's primary axis, specified in global
 * coordinates.
 */
void PhysxJoint::
set_global_axis(const LVector3f &axis) {

  nassertv(_error_type == ET_ok);
  ptr()->setGlobalAxis(PhysxManager::vec3_to_nxVec3(axis));
}

/**
 * Retrieves the joint axis.
 */
LVector3f PhysxJoint::
get_global_axis() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(ptr()->getGlobalAxis());
}

/**
 * Sets the maximum force magnitude that the joint is able to withstand
 * without breaking.
 *
 * If the joint force rises above this threshold, the joint breaks, and
 * becomes disabled.
 *
 * There are two values, one for linear forces, and one for angular forces.
 * Both values are used directly as a value for the maximum impulse tolerated
 * by the joint constraints.
 *
 * Both force values are NX_MAX_REAL by default.  This setting makes the joint
 * unbreakable.  The values should always be nonnegative.
 *
 * The distinction between maxForce and maxTorque is dependent on how the
 * joint is implemented internally, which may not be obvious.  For example
 * what appears to be an angular degree of freedom may be constrained
 * indirectly by a linear constraint.
 *
 * So in most practical applications the user should set both maxTorque and
 * maxForce to low values.
 */
void PhysxJoint::
set_breakable(float maxForce, float maxTorque) {

  nassertv(_error_type == ET_ok);
  ptr()->setBreakable(maxForce, maxTorque);
}

/**
 * Switch between acceleration and force based spring.
 */
void PhysxJoint::
set_use_acceleration_spring(bool value) {

  nassertv(_error_type == ET_ok);
  ptr()->setUseAccelerationSpring(value);
}

/**
 * Checks whether acceleration spring is used.
 */
bool PhysxJoint::
get_use_acceleration_spring() const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->getUseAccelerationSpring();
}

/**
 * Sets the solver extrapolation factor.
 */
void PhysxJoint::
set_solver_extrapolation_factor(float factor) {

  nassertv(_error_type == ET_ok);
  ptr()->setSolverExtrapolationFactor(factor);
}

/**
 * Retrieves the solver extrapolation factor.
 */
float PhysxJoint::
get_solver_extrapolation_factor() const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->getSolverExtrapolationFactor();
}

/**
 * Sets the limit point.  The point is specified in the global coordinate
 * frame.
 *
 * All types of joints may be limited with the same system: You may elect a
 * point attached to one of the two actors to act as the limit point.  You may
 * also specify several planes attached to the other actor.
 *
 * The points and planes move together with the actor they are attached to.
 *
 * The simulation then makes certain that the pair of actors only move
 * relative to each other so that the limit point stays on the positive side
 * of all limit planes.
 *
 * The default limit point is (0,0,0) in the local frame of actor2. Calling
 * this deletes all existing limit planes
 */
void PhysxJoint::
set_limit_point(const LPoint3f &pos, bool isOnActor2) {

  nassertv(_error_type == ET_ok);
  ptr()->setLimitPoint(PhysxManager::point3_to_nxVec3(pos), isOnActor2);
}

/**
 * Adds a limit plane.  The parameters are given in global coordinates.  The
 * plane is affixed to the actor that does not have the limit point.
 *
 * The normal of the plane points toward the positive side of the plane, and
 * thus toward the limit point.  If the normal points away from the limit
 * point at the time of this call, the method returns false and the limit
 * plane is ignored.
 */
void PhysxJoint::
add_limit_plane(const LVector3f &normal, const LPoint3f &pointInPlane, float restitution) {

  nassertv(_error_type == ET_ok);
  ptr()->addLimitPlane(PhysxManager::vec3_to_nxVec3(normal),
                       PhysxManager::point3_to_nxVec3(pointInPlane),
                       restitution);
}

/**
 * Deletes all limit planes added to the joint.
 */
void PhysxJoint::
purge_limit_planes() {

  nassertv(_error_type == ET_ok);
  ptr()->purgeLimitPlanes();
}
