/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBody.cxx
 * @author enn0x
 * @date 2010-09-13
 */

#include "physxSoftBody.h"
#include "physxSoftBodyDesc.h"
#include "physxSoftBodyNode.h"
#include "physxScene.h"
#include "physxGroupsMask.h"

#include "boundingBox.h"

TypeHandle PhysxSoftBody::_type_handle;

/**
 *
 */
void PhysxSoftBody::
link(NxSoftBody *softbodyPtr) {

  // Link self
  _ptr = softbodyPtr;
  _error_type = ET_ok;
  _ptr->userData = this;

  set_name(softbodyPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_softbodies.add(this);
}

/**
 *
 */
void PhysxSoftBody::
unlink() {

  // Unlink self
  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_softbodies.remove(this);

  _node = nullptr;
}

/**
 *
 */
void PhysxSoftBody::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseSoftBody(*_ptr);
  _ptr = nullptr;
}

/**
 *
 */
void PhysxSoftBody::
update() {

  if (_node) {

    // Update node mesh data
    _node->update();

    // Update node bounding volume
    NxBounds3 bounds;
    _ptr->getWorldBounds(bounds);

    BoundingBox bb(PhysxManager::nxVec3_to_point3(bounds.min),
                   PhysxManager::nxVec3_to_point3(bounds.max));
    _node->set_bounds(&bb);
  }
}

/**
 * Returns the scene which this soft body belongs to.
 */
PhysxScene *PhysxSoftBody::
get_scene() const {

  nassertr(_error_type == ET_ok, nullptr);
  return (PhysxScene *)_ptr->getScene().userData;
}

/**
 *
 */
PhysxSoftBodyNode *PhysxSoftBody::
get_soft_body_node() const {

  nassertr(_error_type == ET_ok, nullptr);
  return _node;
}

/**
 *
 */
PhysxSoftBodyNode *PhysxSoftBody::
create_soft_body_node(const char *name) {

  nassertr(_error_type == ET_ok, nullptr);

  _node = new PhysxSoftBodyNode(name);
  _node->allocate(this);

  return _node;
}

/**
 * Sets a name string for the object that can be retrieved with get_name().
 * This is for debugging and is not used by the engine.
 */
void PhysxSoftBody::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  _ptr->setName(_name.c_str());
}

/**
 * Retrieves the name string.
 */
const char *PhysxSoftBody::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return _ptr->getName();
}

/**
 * Sets which collision group this soft body is part of.  Collision group must
 * be between 0 and 31.
 */
void PhysxSoftBody::
set_group(unsigned int group) {

  nassertv(_error_type == ET_ok);
  nassertv(group >= 0 && group < 32);
  _ptr->setGroup(group);
}

/**
 * Retrieves the collision group this soft body is part of.
 */
unsigned int PhysxSoftBody::
get_group() const {

  nassertr(_error_type == ET_ok, 0);
  return _ptr->getGroup();
}

/**
 * Sets 128-bit mask used for collision filtering.
 */
void PhysxSoftBody::
set_groups_mask(const PhysxGroupsMask &mask) {

  nassertv(_error_type == ET_ok);

  NxGroupsMask _mask = mask.get_mask();
  _ptr->setGroupsMask(_mask);
}

/**
 * Gets the 128-bit groups mask used for collision filtering.
 */
PhysxGroupsMask PhysxSoftBody::
get_groups_mask() const {

  PhysxGroupsMask mask;

  nassertr(_error_type == ET_ok, mask);

  NxGroupsMask _mask = _ptr->getGroupsMask();
  mask.set_mask(_mask);

  return mask;
}

/**
 * Gets the number of cloth particles.
 */
unsigned int PhysxSoftBody::
get_num_particles() {

  nassertr(_error_type == ET_ok, 0);
  return _ptr->getNumberOfParticles();
}

/**
 * Sets the soft body particle radius (must be positive).
 */
void PhysxSoftBody::
set_particle_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setParticleRadius(radius);
}

/**
 * Gets the soft body particle radius.
 */
float PhysxSoftBody::
get_particle_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getParticleRadius();
}

/**
 * Sets the value of a single flag.
 */
void PhysxSoftBody::
set_flag(PhysxSoftBodyFlag flag, bool value) {

  nassertv(_error_type == ET_ok);

  NxU32 flags = _ptr->getFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  _ptr->setFlags(flags);
}

/**
 * Retrieves the value of a single flag.
 */
bool PhysxSoftBody::
get_flag(PhysxSoftBodyFlag flag) const {

  nassertr(_error_type == ET_ok, false);

  return (_ptr->getFlags() & flag) ? true : false;
}

/**
 * Gets the soft body density.
 */
float PhysxSoftBody::
get_density() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getDensity();
}

/**
 * Gets the relative grid spacing for the broad phase.  The cloth is
 * represented by a set of world aligned cubical cells in broad phase.  The
 * size of these cells is determined by multiplying the length of the diagonal
 * of the AABB of the initial soft body size with this constant.
 */
float PhysxSoftBody::
get_relative_grid_spacing() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRelativeGridSpacing();
}

/**
 * Sets the soft body volume stiffness in the range from 0 to 1.
 */
void PhysxSoftBody::
set_volume_stiffness(float stiffness) {

  nassertv(_error_type == ET_ok);
  ptr()->setVolumeStiffness(stiffness);
}

/**
 * Retrieves the soft body volume stiffness.
 */
float PhysxSoftBody::
get_volume_stiffness() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getVolumeStiffness();
}

/**
 * Sets the soft body stretching stiffness in the range from 0 to 1.
 */
void PhysxSoftBody::
set_stretching_stiffness(float stiffness) {

  nassertv(_error_type == ET_ok);
  ptr()->setStretchingStiffness(stiffness);
}

/**
 * Retrieves the soft body stretching stiffness.
 */
float PhysxSoftBody::
get_stretching_stiffness() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getStretchingStiffness();
}

/**
 * Sets the damping coefficient in the range from 0 to 1.
 */
void PhysxSoftBody::
set_damping_coefficient(float coef) {

  nassertv(_error_type == ET_ok);
  ptr()->setDampingCoefficient(coef);
}

/**
 * Retrieves the damping coefficient.
 */
float PhysxSoftBody::
get_damping_coefficient() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getDampingCoefficient();
}

/**
 * Sets the soft body friction coefficient in the range from 0 to 1.
 */
void PhysxSoftBody::
set_friction(float friction) {

  nassertv(_error_type == ET_ok);
  ptr()->setFriction(friction);
}

/**
 * Retrieves the soft body friction coefficient.
 */
float PhysxSoftBody::
get_friction() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getFriction();
}

/**
 * Sets the soft body tear factor (must be larger than one).
 */
void PhysxSoftBody::
set_tear_factor(float factor) {

  nassertv(_error_type == ET_ok);
  nassertv(factor > 1.0f);
  ptr()->setTearFactor(factor);
}

/**
 * Retrieves the soft body tear factor.
 */
float PhysxSoftBody::
get_tear_factor() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getTearFactor();
}

/**
 * Sets the soft body attachment tear factor (must be larger than one).
 */
void PhysxSoftBody::
set_attachment_tear_factor(float factor) {

  nassertv(_error_type == ET_ok);
  nassertv(factor > 1.0f);
  ptr()->setAttachmentTearFactor(factor);
}

/**
 * Retrieves the attachment soft body tear factor.
 */
float PhysxSoftBody::
get_attachment_tear_factor() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getAttachmentTearFactor();
}

/**
 * Sets the soft body solver iterations.
 */
void PhysxSoftBody::
set_solver_iterations(unsigned int iterations) {

  nassertv(_error_type == ET_ok);
  ptr()->setSolverIterations(iterations);
}

/**
 * Retrieves the soft body solver iterations.
 */
unsigned int PhysxSoftBody::
get_solver_iterations() const {

  nassertr(_error_type == ET_ok, 0);
  return ptr()->getSolverIterations();
}

/**
 * Returns true if this soft body is sleeping.
 *
 * When a soft body does not move for a period of time, it is no longer
 * simulated in order to save time.  This state is called sleeping.  However,
 * because the object automatically wakes up when it is either touched by an
 * awake object, or one of its properties is changed by the user, the entire
 * sleep mechanism should be transparent to the user.
 */
bool PhysxSoftBody::
is_sleeping() const {

  nassertr(_error_type == ET_ok, false);
  return _ptr->isSleeping();
}

/**
 * Wakes up the soft body if it is sleeping.
 *
 * The wakeCounterValue determines how long until the body is put to sleep, a
 * value of zero means that the body is sleeping.  wake_up(0) is equivalent to
 * PhysxSoftBody::put_to_sleep().
 */
void PhysxSoftBody::
wake_up(float wakeCounterValue) {

  nassertv(_error_type == ET_ok);
  _ptr->wakeUp(wakeCounterValue);
}

/**
 * Forces the soft body to sleep.
 *
 * The soft body  will stay asleep until the next call to simulate, and will
 * not wake up until then even when otherwise it would (for example a force is
 * applied to it). It can however wake up during the next do_physics call.
 */
void PhysxSoftBody::
put_to_sleep() {

  nassertv(_error_type == ET_ok);
  _ptr->putToSleep();
}

/**
 * Sets the linear velocity below which an soft body may go to sleep.
 * SoftBodys whose linear velocity is above this threshold will not be put to
 * sleep.
 *
 * Setting the sleep angular/linear velocity only makes sense when the
 * BF_energy_sleep_test is not set.
 */
void PhysxSoftBody::
set_sleep_linear_velocity(float threshold) {

  nassertv(_error_type == ET_ok);
  _ptr->setSleepLinearVelocity(threshold);
}

/**
 * Returns the linear velocity below which an soft body may go to sleep.  Soft
 * bodies whose linear velocity is above this threshold will not be put to
 * sleep.
 */
float PhysxSoftBody::
get_sleep_linear_velocity() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getSleepLinearVelocity();
}

#if NX_SDK_VERSION_NUMBER > 281
/**
 * Sets the soft body self collision thickness (must be positive).
 */
void PhysxSoftBody::
set_self_collision_thickness(float thickness) {

  nassertv(_error_type == ET_ok);
  _ptr->setSelfCollisionThickness(thickness);
}

/**
 * Gets the soft body self collision thickness.
 */
float PhysxSoftBody::
get_self_collision_thickness() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getSelfCollisionThickness();
}

/**
 * Sets the soft body hard stretch elongation limit.
 */
void PhysxSoftBody::
set_hard_stretch_limitation_factor(float factor) {

  nassertv(_error_type == ET_ok);
  ptr()->setHardStretchLimitationFactor(factor);
}

/**
 * Retrieves the soft body hard stretch elongation limit.
 */
float PhysxSoftBody::
get_hard_stretch_limitation_factor() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return ptr()->getHardStretchLimitationFactor();
}
#endif // NX_SDK_VERSION_NUMBER > 281


/**
 * Attaches a cloth vertex to a position in world space.
 */
/*
void PhysxSoftBody::
attach_vertex_to_global_pos(unsigned int vertexId, LPoint3f const &pos) {

  nassertv(_error_type == ET_ok);
  nassertv(!pos.is_nan());

  _ptr->attachVertexToGlobalPosition(vertexId, PhysxManager::point3_to_nxVec3(pos));
}
*/

/**
 * Attaches the cloth to a shape.  All cloth points currently inside the shape
 * are attached.
 *
 * This method only works with primitive and convex shapes.  Since the inside
 * of a general triangle mesh is not clearly defined.
 */
/*
void PhysxSoftBody::
attach_to_shape(PhysxShape *shape) {

  nassertv(_error_type == ET_ok);
  nassertv(shape);

  NxU32 attachmentFlags = 0; // --TODO--
  _ptr->attachToShape(shape->ptr(), attachmentFlags);
}
*/

/**
 * Attaches the cloth to all shapes, currently colliding.
 *
 * This method only works with primitive and convex shapes.  Since the inside
 * of a general triangle mesh is not clearly defined.
 */
/*
void PhysxSoftBody::
attach_to_colliding_shapes() {

  nassertv(_error_type == ET_ok);

  NxU32 attachmentFlags = 0; // --TODO--
  _ptr->attachToCollidingShapes(attachmentFlags);
}
*/

/**
 * Detaches the cloth from a shape it has been attached to before.
 *
 * If the cloth has not been attached to the shape before, the call has no
 * effect.
 */
/*
void PhysxSoftBody::
detach_from_shape(PhysxShape *shape) {

  nassertv(_error_type == ET_ok);
  nassertv(shape);

  _ptr->detachFromShape(shape->ptr());
}
*/

/**
 * Frees a previously attached cloth point.
 */
/*
void PhysxSoftBody::
free_vertex(unsigned int vertexId) {

  nassertv(_error_type == ET_ok);
  _ptr->freeVertex(vertexId);
}
*/

/**
 * Attaches a cloth vertex to a local position within a shape.
 */
/*
void PhysxSoftBody::
attach_vertex_to_shape(unsigned int vertexId, PhysxShape *shape, LPoint3f const &localPos) {

  nassertv(_error_type == ET_ok);
  nassertv(!localPos.is_nan());
  nassertv(shape);

  NxU32 attachmentFlags = 0; // --TODO--
  _ptr->attachVertexToShape(vertexId, shape->ptr(),
                            PhysxManager::point3_to_nxVec3(localPos),
                            attachmentFlags);
}
*/

/**
 * Return the attachment status of the given vertex.
 */
/*
PhysxEnums::PhysxVertexAttachmentStatus PhysxSoftBody::
get_vertex_attachment_status(unsigned int vertexId) const {

  nassertr(_error_type == ET_ok, VAS_none);
  // --TODO-- nassertr(vertexId < _ptr->getNumberOfParticles(), VAS_none);

  return (PhysxVertexAttachmentStatus) _ptr->getVertexAttachmentStatus(vertexId);
}
*/

/**
 * Returns the pointer to an attached shape pointer of the given vertex.  If
 * the vertex is not attached or attached to a global position, NULL is
 * returned.
 */
/*
PhysxShape *PhysxSoftBody::
get_vertex_attachment_shape(unsigned int vertexId) const {

  nassertr(_error_type == ET_ok, NULL);
  // --TODO-- nassertr(vertexId < _ptr->getNumberOfParticles(), NULL);

  NxShape *shapePtr = _ptr->getVertexAttachmentShape(vertexId);
  PhysxShape *shape = shapePtr ? (PhysxShape *)(shapePtr->userData) : NULL;

  return shape;
}
*/

/**
 * Returns the attachment position of the given vertex.  If the vertex is
 * attached to shape, the position local to the shape's pose is returned.  If
 * the vertex is not attached, the return value is undefined.
 */
/*
LPoint3f PhysxSoftBody::
get_vertex_attachment_pos(unsigned int vertexId) const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());
  // --TODO-- nassertr(vertexId < _ptr->getNumberOfParticles(),
  // LPoint3f::zero());

  return PhysxManager::nxVec3_to_point3(_ptr->getVertexAttachmentPosition(vertexId));
}
*/

/**
 * Sets an external acceleration which affects all non attached particles of
 * the cloth.
 */
/*
void PhysxSoftBody::
set_external_acceleration(LVector3f const &acceleration) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!acceleration.is_nan());

  _ptr->setExternalAcceleration(PhysxManager::vec3_to_nxVec3(acceleration));
}
*/

/**
 * Sets an acceleration acting normal to the cloth surface at each vertex.
 */
/*
void PhysxSoftBody::
set_wind_acceleration(LVector3f const &acceleration) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!acceleration.is_nan());

  _ptr->setWindAcceleration(PhysxManager::vec3_to_nxVec3(acceleration));
}
*/

/**
 * Retrieves the external acceleration which affects all non attached
 * particles of the cloth.
 */
/*
LVector3f PhysxSoftBody::
get_external_acceleration() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getExternalAcceleration());
}
*/

/**
 * Retrieves the acceleration acting normal to the cloth surface at each
 * vertex
 */
/*
LVector3f PhysxSoftBody::
get_wind_acceleration() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getWindAcceleration());
}
*/

/**
 * Applies a force (or impulse) defined in the global coordinate frame, to a
 * particular vertex of the cloth.
 */
/*
void PhysxSoftBody::
add_force_at_vertex(LVector3f const &force, int vertexId, PhysxForceMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->addForceAtVertex(PhysxManager::vec3_to_nxVec3(force),
                         vertexId,
                         (NxForceMode) mode);
}
*/

/**
 * Applies a radial force (or impulse) at a particular position.  All vertices
 * within radius will be affected with a quadratic drop-off.
 */
/*
void PhysxSoftBody::
add_force_at_pos(LPoint3f const &pos, float magnitude, float radius, PhysxForceMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->addForceAtPos(PhysxManager::point3_to_nxVec3(pos),
                      magnitude,
                      radius,
                      (NxForceMode) mode);
}
*/

/**
 * Applies a directed force (or impulse) at a particular position.  All
 * vertices within radius will be affected with a quadratic drop-off.
 */
/*
void PhysxSoftBody::
add_directed_force_at_pos(LPoint3f const &pos, LVector3f const &force, float radius, PhysxForceMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->addDirectedForceAtPos(PhysxManager::point3_to_nxVec3(pos),
                              PhysxManager::vec3_to_nxVec3(force),
                              radius,
                              (NxForceMode) mode);
}
*/
