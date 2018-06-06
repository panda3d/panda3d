/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxActor.cxx
 * @author enn0x
 * @date 2009-09-14
 */

#include "physxActor.h"
#include "physxActorDesc.h"
#include "physxBodyDesc.h"
#include "physxShapeDesc.h"
#include "physxManager.h"

TypeHandle PhysxActor::_type_handle;

/**
 *
 */
void PhysxActor::
link(NxActor *actorPtr) {

  // Link self
  _ptr = actorPtr;
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(actorPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_actors.add(this);

  // Link shapes
  NxShape * const *shapes = _ptr->getShapes();
  NxU32 nShapes = _ptr->getNbShapes();

  for (NxU32 i=0; i < nShapes; i++) {
    PhysxShape *shape = PhysxShape::factory(shapes[i]->getType());
    shape->link(shapes[i]);
  }
}

/**
 *
 */
void PhysxActor::
unlink() {

  // Unlink shapes
  NxShape * const *shapes = _ptr->getShapes();
  NxU32 nShapes = _ptr->getNbShapes();

  for (NxU32 i=0; i < nShapes; i++) {
    PhysxShape *shape = (PhysxShape *)shapes[i]->userData;
    shape->unlink();
  }

  // Unlink self
  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_actors.remove(this);
}

/**
 *
 */
void PhysxActor::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseActor(*_ptr);
  _ptr = nullptr;
}

/**
 *
 */
void PhysxActor::
link_controller(PhysxController *controller) {

  _controller = controller;
}

/**
 * Saves the body information of a dynamic actor to the passed body
 * descriptor.
 */
bool PhysxActor::
save_body_to_desc(PhysxBodyDesc &bodyDesc) const {

  nassertr(_error_type == ET_ok, false);
  return _ptr->saveBodyToDesc(bodyDesc._desc);
}

/**
 * Saves the state of the actor to the passed descriptor.
 */
void PhysxActor::
save_to_desc(PhysxActorDesc &actorDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(actorDesc._desc);
}

/**
 * Sets a name string for the object that can be retrieved with get_name().
 * This is for debugging and is not used by the engine.
 */
void PhysxActor::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  _ptr->setName(_name.c_str());
}

/**
 * Retrieves the name string.
 */
const char *PhysxActor::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return _ptr->getName();
}

/**
 * Updates the transform of an assigned NodePath.  If the actor has been
 * created by a PhysxController then this method will update the NodePath's
 * transform from the controller's transform.
 */
void PhysxActor::
update_transform(const LMatrix4f &m) {

  // Active transforms are update AFTER scene.fetchResults() has been called,
  // and thus can contain removed objects.  So either update transforms after
  // scene.fetchResults() - which means poor performance - or check if an
  // actor has been removed here in this method.
  if (_error_type != ET_ok) return;

  if (_np.is_empty()) return;

  if (_controller) {
    LVector3f hpr(_controller->get_h(), 0.0f, 0.0f);
    LPoint3f pos = _controller->get_pos();
    _np.set_transform(_np.get_top(), TransformState::make_pos_hpr(pos, hpr));
  }
  else {
    _np.set_transform(_np.get_top(), TransformState::make_mat(m));
  }
}

/**
 * Retrieves the actors world space position.
 */
LPoint3f PhysxActor::
get_global_pos() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());
  return PhysxManager::nxVec3_to_point3(_ptr->getGlobalPosition());
}

/**
 * Retrieves the actors world space transform.
 */
LMatrix4f PhysxActor::
get_global_mat() const {

  nassertr(_error_type == ET_ok, LMatrix4f::zeros_mat());
  return PhysxManager::nxMat34_to_mat4(_ptr->getGlobalPose());
}

/**
 * Retrieves the actors world space orientation.
 */
LQuaternionf PhysxActor::
get_global_quat() const {

  nassertr(_error_type == ET_ok, LQuaternionf::zero());
  return PhysxManager::nxQuat_to_quat(_ptr->getGlobalOrientation());
}

/**
 * Method for setting a dynamic actor's position in the world.  Please see
 * set_global_mat for some caveats.
 */
void PhysxActor::
set_global_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!pos.is_nan());

  _ptr->setGlobalPosition(PhysxManager::point3_to_nxVec3(pos));
}

/**
 * Method for setting a dynamic actor's transform matrix in the world.
 *
 * This method instantaneously changes the actor space to world space
 * transformation.
 *
 * One should exercise restraint in making use of these methods.
 *
 * Static actors should not be moved at all.  There are various internal data
 * structures for static actors which may need to be recomputed when one
 * moves.  Also, moving static actors will not interact correctly with dynamic
 * actors or joints.  If you would like to directly control an actor's
 * position and would like to have it correctly interact with dynamic bodies
 * and joints, you should create a dynamic body with the BF_kinematic flag,
 * and then use the move_global_*() commands to move it along a path!
 *
 * When briefly moving dynamic actors, one should not: - Move actors into
 * other actors, thus causing interpenetration (an invalid physical state). -
 * Move an actor that is connected by a joint to another away from the other
 * (thus causing joint error). - When moving jointed actors the joints' cached
 * transform information is destroyed and recreated next frame; thus this call
 * is expensive for jointed actors.
 */
void PhysxActor::
set_global_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!mat.is_nan());

  _ptr->setGlobalPose(PhysxManager::mat4_to_nxMat34(mat));
}

/**
 * Method for setting a dynamic actor's orientation in the world.  Please see
 * set_global_mat for some caveats.
 */
void PhysxActor::
set_global_hpr(float h, float p, float r) {

  nassertv(_error_type == ET_ok);

  LQuaternionf q;
  q.set_hpr(LVector3f(h, p, r));
  _ptr->setGlobalOrientationQuat(PhysxManager::quat_to_nxQuat(q));
}

/**
 * The move_global_* calls serve to move kinematically controlled dynamic
 * actors through the game world.
 *
 * See move_global_mat() for more information.
 *
 * This call wakes the actor if it is sleeping.
 */
void PhysxActor::
move_global_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!pos.is_nan());

  _ptr->moveGlobalPosition(PhysxManager::point3_to_nxVec3(pos));
}

/**
 * The move_global_* calls serve to move kinematically controlled dynamic
 * actors through the game world.
 *
 * You set a dynamic actor to be kinematic using the BF_KINEMATIC body flag,
 * used either in the PhysBodyDesc or with set_body_flag().
 *
 * The move command will result in a velocity that, when successfully carried
 * out (i.e.  the motion is not blocked due to joints or collisions) inside
 * run*(), will move the body into the desired pose.  After the move is
 * carried out during a single time step, the velocity is returned to zero.
 * Thus, you must continuously call this in every time step for kinematic
 * actors so that they move along a path.
 *
 * These functions simply store the move destination until run*() is called,
 * so consecutive calls will simply overwrite the stored target variable.
 *
 * This call wakes the actor if it is sleeping.
 */
void PhysxActor::
move_global_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!mat.is_nan());

  _ptr->moveGlobalPose(PhysxManager::mat4_to_nxMat34(mat));
}

/**
 * The move_global_* calls serve to move kinematically controlled dynamic
 * actors through the game world.
 *
 * See move_global_mat() for more information.
 *
 * This call wakes the actor if it is sleeping.
 */
void PhysxActor::
move_global_hpr(float h, float p, float r) {

  nassertv(_error_type == ET_ok);

  LQuaternionf q;
  q.set_hpr(LVector3f(h, p, r));
  _ptr->moveGlobalOrientationQuat(PhysxManager::quat_to_nxQuat(q));
}

/**
 * Attaches a node path to this actor.  The node path's transform will be
 * updated automatically if the actor's transform changes (and only then).
 *
 * Note: any non-uniform scale or shear set on the NodePath's transform will
 * be overwritten at the time of the first update.
 */
void PhysxActor::
attach_node_path(const NodePath &np) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!np.is_empty());

  _np = NodePath(np);
}

/**
 * Detaches a previously assigned NodePath from this actor.  The NodePath's
 * transform will no longer be updated from the actor's transform.
 */
void PhysxActor::
detach_node_path() {

  nassertv(_error_type == ET_ok);

  _np = NodePath();
}

/**
 * Retrieves a previously attached NodePath.  An empty NodePath will be
 * returned if no NodePath has been attached to this actor.
 */
NodePath PhysxActor::
get_node_path() const {

  nassertr(_error_type == ET_ok, NodePath::fail());

  return _np;
}

/**
 * Retrieves the scene which this actor belongs to.
 */
PhysxScene *PhysxActor::
get_scene() const {

  nassertr(_error_type == ET_ok, nullptr);

  NxScene *scenePtr = &(_ptr->getScene());
  PhysxScene *scene = (PhysxScene *)(scenePtr->userData);

  return scene;
}

/**
 * Returns the number of shapes assigned to the actor.
 */
unsigned int PhysxActor::
get_num_shapes() const {

  nassertr(_error_type == ET_ok, -1);

  return _ptr->getNbShapes();
}

/**
 * Creates a new shape and adds it to the list of shapes of this actor.
 *
 * Mass properties of dynamic actors will not automatically be recomputed to
 * reflect the new mass distribution implied by the shape.  Follow this call
 * with a call to update_mass_from_shapes() to do that.
 */
PhysxShape *PhysxActor::
create_shape(PhysxShapeDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(),nullptr);

  PhysxShape *shape = PhysxShape::factory(desc.ptr()->getType());
  nassertr(shape, nullptr);

  NxShape *shapePtr = _ptr->createShape(*desc.ptr());
  nassertr(shapePtr, nullptr);

  shape->link(shapePtr);

  return shape;
}

/**
 * Retrieves an individual shape from the actor's array of shapes.  Index must
 * be in the range from zero to (number-of-shapes minus 1).
 */
PhysxShape *PhysxActor::
get_shape(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbShapes(), nullptr);

  NxShape * const *shapes = _ptr->getShapes();
  NxShape *shapePtr = shapes[idx];
  PhysxShape *shape = (PhysxShape *)(shapePtr->userData);

  return shape;
}

/**
 * Retrieves an individual shape from the actor's array of shapes.  The first
 * shape for which the shape's name matches the specified name is returned, or
 * NULL if no shape has a matching name.
 */
PhysxShape *PhysxActor::
get_shape_by_name(const char *name) const {

  nassertr(_error_type == ET_ok, nullptr);

  NxShape * const *shapes = _ptr->getShapes();
  NxShape *shapePtr = nullptr;
  NxU32 nShapes = _ptr->getNbShapes();

  for (NxU32 i=0; i < nShapes; i++) {
    shapePtr = shapes[i];

    if (strcmp(shapePtr->getName(), name) == 0) {
      return (PhysxShape *) shapePtr->userData;
    }
  }

  return nullptr;
}

/**
 * Applies a force (or impulse) defined in the global coordinate frame to the
 * actor.
 *
 * This will not induce a torque.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_force(const LVector3f force, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!force.is_nan());

  _ptr->addForce(PhysxManager::vec3_to_nxVec3(force), (NxForceMode)mode, wakeup);
}

/**
 * Applies a force (or impulse) defined in the global coordinate frame, acting
 * at a particular point in global coordinates, to the actor.
 *
 * Note that if the force does not act along the center of mass of the actor,
 * this will also add the corresponding torque.  Because forces are reset at
 * the end of every timestep, you can maintain a total external force on an
 * object by calling this once every frame.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_force_at_pos(const LVector3f force, const LPoint3f &pos, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!force.is_nan());
  nassertv_always(!pos.is_nan());

  _ptr->addForceAtPos(PhysxManager::vec3_to_nxVec3(force), PhysxManager::point3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

/**
 * Applies a force (or impulse) defined in the global coordinate frame, acting
 * at a particular point in local coordinates, to the actor.
 *
 * Note that if the force does not act along the center of mass of the actor,
 * this will also add the corresponding torque.  Because forces are reset at
 * the end of every timestep, you can maintain a total external force on an
 * object by calling this once every frame.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_force_at_local_pos(const LVector3f force, const LPoint3f &pos, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!force.is_nan());
  nassertv_always(!pos.is_nan());

  _ptr->addForceAtLocalPos(PhysxManager::vec3_to_nxVec3(force), PhysxManager::point3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

/**
 * Applies an impulsive torque defined in the global coordinate frame to the
 * actor.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_torque(const LVector3f torque, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!torque.is_nan());

  _ptr->addTorque(PhysxManager::vec3_to_nxVec3(torque), (NxForceMode)mode, wakeup);
}

/**
 * Applies a force (or impulse) defined in the actor local coordinate frame to
 * the actor.  This will not induce a torque.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_local_force(const LVector3f force, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!force.is_nan());

  _ptr->addLocalForce(PhysxManager::vec3_to_nxVec3(force), (NxForceMode)mode, wakeup);
}

/**
 * Applies a force (or impulse) defined in the actor local coordinate frame,
 * acting at a particular point in global coordinates, to the actor.
 *
 * Note that if the force does not act along the center of mass of the actor,
 * this will also add the corresponding torque.  Because forces are reset at
 * the end of every timestep, you can maintain a total external force on an
 * object by calling this once every frame.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_local_force_at_pos(const LVector3f force, const LPoint3f &pos, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!force.is_nan());
  nassertv_always(!pos.is_nan());

  _ptr->addLocalForceAtPos(PhysxManager::vec3_to_nxVec3(force), PhysxManager::point3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

/**
 * Applies a force (or impulse) defined in the actor local coordinate frame,
 * acting at a particular point in local coordinates, to the actor.
 *
 * Note that if the force does not act along the center of mass of the actor,
 * this will also add the corresponding torque.  Because forces are reset at
 * the end of every timestep, you can maintain a total external force on an
 * object by calling this once every frame.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_local_force_at_local_pos(const LVector3f force, const LPoint3f &pos, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!force.is_nan());
  nassertv_always(!pos.is_nan());

  _ptr->addLocalForceAtLocalPos(PhysxManager::vec3_to_nxVec3(force), PhysxManager::point3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

/**
 * Applies an impulsive torque defined in the actor local coordinate frame to
 * the actor.
 *
 * Mode determines if the torque is to be conventional or impulsive.
 *
 * The actor must be dynamic.  This call wakes the actor if it is sleeping and
 * the wakeup parameter is true (default).
 */
void PhysxActor::
add_local_torque(const LVector3f torque, PhysxForceMode mode, bool wakeup) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!torque.is_nan());

  _ptr->addLocalTorque(PhysxManager::vec3_to_nxVec3(torque), (NxForceMode)mode, wakeup);
}

/**
 * Recomputes a dynamic actor's mass properties from its shapes.
 *
 * Given a constant density or total mass, the actors mass properties can be
 * recomputed using the shapes attached to the actor.  If the actor has no
 * shapes, then only the totalMass parameter can be used.  If all shapes in
 * the actor are trigger shapes (non-physical), the call will fail.
 *
 * The mass of each shape is either the shape's local density (as specified in
 * the PhysxShapeDesc; default 1.0) multiplied by the shape's volume or a
 * directly specified shape mass.
 *
 * The inertia tensor, mass frame and center of mass will always be
 * recomputed.  If there are no shapes in the actor, the mass will be
 * totalMass, and the mass frame will be set to the center of the actor.
 *
 * If you supply a non-zero total mass, the actor's mass and inertia will
 * first be computed as above and then scaled to fit this total mass.
 *
 * If you supply a non-zero density, the actor's mass and inertia will first
 * be computed as above and then scaled by this factor.
 *
 * Either totalMass or density must be non-zero.
 *
 * The actor must be dynamic.
 */
bool PhysxActor::
update_mass_from_shapes(float density, float totalMass) {

  nassertr(_error_type == ET_ok, false);
  return _ptr->updateMassFromShapes(density, totalMass);
}

/**
 * Computes the total kinetic (rotational and translational) energy of the
 * object.  The actor must be dynamic.
 */
float PhysxActor::
compute_kinetic_energy() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->computeKineticEnergy();
}

/**
 * Returns true if the actor is dynamic.
 */
bool PhysxActor::
is_dynamic() const {

  nassertr(_error_type == ET_ok, false);
  return _ptr->isDynamic();
}

/**
 * Sets the collision group for all shapes of this actor.  See
 * PhysxShape.setGroup().
 */
void PhysxActor::
set_shape_group(unsigned int group) {

  nassertv(_error_type == ET_ok);
  nassertv(group >= 0 && group < 32);

  NxShape * const *shapes = _ptr->getShapes();
  NxU32 nShapes = _ptr->getNbShapes();

  for (NxU32 i=0; i < nShapes; i++) {
    shapes[i]->setGroup( group );
  }
}

/**
 * Raise or lower individual BodyFlag flags.
 */
void PhysxActor::
set_body_flag(PhysxBodyFlag flag, bool value) {

  if (value == true) {
    _ptr->raiseBodyFlag((NxBodyFlag)flag);
  }
  else {
    _ptr->clearBodyFlag((NxBodyFlag)flag);
  }
}

/**
 * Return the specified BodyFlag flag.
 */
bool PhysxActor::
get_body_flag(PhysxBodyFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->readBodyFlag((NxBodyFlag)flag);
}

/**
 * Raise or lower individual ActorFlag flags.
 */
void PhysxActor::
set_actor_flag(PhysxActorFlag flag, bool value) {

  if (value == true) {
    _ptr->raiseActorFlag((NxActorFlag)flag);
  }
  else {
    _ptr->clearActorFlag((NxActorFlag)flag);
  }
}

/**
 * Return the specified ActorFlag flag.
 */
bool PhysxActor::
get_actor_flag(PhysxActorFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->readActorFlag((NxActorFlag)flag);
}

/**
 * Sets the actor's contact report flags.
 *
 * These flags are used to determine the kind of report that is generated for
 * interactions with other actors.
 *
 * Please note: If the actor is part of an interacting pair for which the
 * contact report generation is controlled already through any other mechanism
 * (for example by use of PhysxScene::set_actor_pair_flags) then the union of
 * all the specified contact report flags will be used to generate the report.
 */
void PhysxActor::
set_contact_report_flag(PhysxContactPairFlag flag, bool value) {

  nassertv(_error_type == ET_ok);

  NxU32 flags = _ptr->getContactReportFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  _ptr->setContactReportFlags(flags);
}

/**
 * Sets the force threshold for contact reports.  The actor must be dynamic.
 */
void PhysxActor::
set_contact_report_threshold(float threshold) {

  nassertv(_error_type == ET_ok);
  nassertv(threshold >= 0.0f);

  _ptr->setContactReportThreshold(threshold);
}

/**
 * Assigns the actor to a user defined group of actors.  The actor group must
 * be an integer in between 0 and 0x7fff (32767).
 *
 * This is similar to NxShape groups, except those are only five bits and
 * serve a different purpose.
 *
 * The PhysxScene::set_actor_group_pair_flags() lets you set certain behaviors
 * for pairs of actor groups.
 *
 * By default every actor is created in group 0.
 */
void PhysxActor::
set_group(unsigned int group) {

  nassertv(_error_type == ET_ok);
  nassertv(group >= 0 && group < 0x8000);

  ptr()->setGroup(group);
}

/**
 * Retrieves the actor group this actor is assigned to.
 */
unsigned int PhysxActor::
get_group() const {

  nassertr(_error_type == ET_ok, 0);

  return ptr()->getGroup();
}

/**
 * Assigns dynamic actors a dominance group identifier.  Dominance groups are
 * integere in the range from 0 to 31.
 *
 * This is similar to shape groups, except those serve a different purpose.
 *
 * The PhysxScene::set_dominance_group_pair() lets you set certain behaviors
 * for pairs of dominance groups.
 *
 * By default every actor is created in group 0. Static actors must stay in
 * group 0; thus you can only call this on dynamic actors.
 */
void PhysxActor::
set_dominance_group(unsigned int group) {

  nassertv(_error_type == ET_ok);
  nassertv(group >= 0 && group < 32);
  nassertv(is_dynamic() == true);

  _ptr->setDominanceGroup(group);
}

/**
 * Retrieves the dominance group of this actor.
 */
unsigned int PhysxActor::
get_dominance_group() const {

  nassertr(_error_type == ET_ok, 0);

  return ptr()->getDominanceGroup();
}

/**
 * Sets the angular damping coefficient.  Zero represents no damping.  The
 * angular damping coefficient must be nonnegative.  The actor must be
 * dynamic.  Default: 0.05
 */
void PhysxActor::
set_angular_damping(float angDamp) {

  nassertv(_error_type == ET_ok);
  nassertv(angDamp >= 0.0f);

  _ptr->setAngularDamping(angDamp);
}

/**
 * Returns the angular damping coefficient.  The actor must be dynamic.
 */
float PhysxActor::
get_angular_damping() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getAngularDamping();
}

/**
 * Sets the linear damping coefficient.  Zero represents no damping.  The
 * damping coefficient must be nonnegative.  The actor must be dynamic.
 * Default: 0
 */
void PhysxActor::
set_linear_damping(float linDamp) {

  nassertv(_error_type == ET_ok);
  nassertv(linDamp >= 0.0f);

  _ptr->setLinearDamping(linDamp);
}

/**
 * Retrieves the linear damping coefficient.  The actor must be dynamic.
 */
float PhysxActor::
get_linear_damping() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getLinearDamping();
}

/**
 * Sets the linear velocity of the actor.
 *
 * Note that if you continuously set the velocity of an actor yourself, forces
 * such as gravity or friction will not be able to manifest themselves,
 * because forces directly influence only the velocity/momentum of an actor.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
set_linear_velocity(const LVector3f &linVel) {

  nassertv(_error_type == ET_ok);
  nassertv(_ptr->isDynamic());

  _ptr->setLinearVelocity(PhysxManager::vec3_to_nxVec3(linVel));
}

/**
 * Sets the angular velocity of the actor.
 *
 * Note that if you continuously set the angular velocity of an actor
 * yourself, forces such as friction will not be able to rotate the actor,
 * because forces directly influence only the velocity/momentum.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
set_angular_velocity(const LVector3f &angVel) {

  nassertv(_error_type == ET_ok);
  nassertv(_ptr->isDynamic());

  _ptr->setAngularVelocity(PhysxManager::vec3_to_nxVec3(angVel));
}

/**
 * Lets you set the maximum angular velocity permitted for this actor.
 *
 * Because for various internal computations, very quickly rotating actors
 * introduce error into the simulation, which leads to undesired results.
 *
 * With PhysxManager::set_parameter(PP_max_angular_velocity) you can set the
 * default maximum velocity for actors created after the call.  Bodies' high
 * angular velocities are clamped to this value.
 *
 * However, because some actors, such as car wheels, should be able to rotate
 * quickly, you can override the default setting on a per-actor basis with the
 * below call.  Note that objects such as wheels which are approximated with
 * spherical or other smooth collision primitives can be simulated with
 * stability at a much higher angular velocity than, say, a box that has
 * corners.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
set_max_angular_velocity(float maxAngVel) {

  nassertv(_error_type == ET_ok);
  nassertv(_ptr->isDynamic());

  _ptr->setMaxAngularVelocity(maxAngVel);
}

/**
 * Returns the linear velocity of an actor.  The actor must be dynamic.
 */
LVector3f PhysxActor::
get_linear_velocity() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getLinearVelocity());
}

/**
 * Returns the angular velocity of the actor.  The actor must be dynamic.
 */
LVector3f PhysxActor::
get_angular_velocity() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getAngularVelocity());
}

/**
 * Returns the maximum angular velocity permitted for this actor.
 */
float PhysxActor::
get_max_angular_velocity() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getMaxAngularVelocity();
}

/**
 * Computes the velocity of a point given in world coordinates if it were
 * attached to the actor and moving with it.
 *
 * The actor must be dynamic.
 */
LVector3f PhysxActor::
get_point_velocity(const LPoint3f &point) const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  nassertr_always(!point.is_nan(), LVector3f::zero());

  NxVec3 nPoint = PhysxManager::point3_to_nxVec3(point);
  return PhysxManager::nxVec3_to_vec3(_ptr->getPointVelocity(nPoint));
}

/**
 * Computes the velocity of a point given in body local coordinates as if it
 * were attached to the actor and moving with it.
 *
 * The actor must be dynamic.
 */
LVector3f PhysxActor::
get_local_point_velocity(const LPoint3f &point) const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  nassertr_always(!point.is_nan(), LVector3f::zero());

  NxVec3 nPoint = PhysxManager::point3_to_nxVec3(point);
  return PhysxManager::nxVec3_to_vec3(_ptr->getLocalPointVelocity(nPoint));
}

/**
 * Sets the linear momentum of the actor.  Note that if you continuously set
 * the linear momentum of an actor yourself, forces such as gravity or
 * friction will not be able to manifest themselves, because forces directly
 * influence only the velocity/momentum of a actor.  The actor must be
 * dynamic.
 */
void PhysxActor::
set_linear_momentum(const LVector3f &momentum) {

  nassertv(_error_type == ET_ok);
  _ptr->setLinearMomentum(PhysxManager::vec3_to_nxVec3(momentum));
}

/**
 * Sets the angular momentum of the actor.  Note that if you continuously set
 * the angular velocity of an actor yourself, forces such as friction will not
 * be able to rotate the actor, because forces directly influence only the
 * velocity of actor.  The actor must be dynamic.
 */
void PhysxActor::
set_angular_momentum(const LVector3f &momentum) {

  nassertv(_error_type == ET_ok);
  _ptr->setAngularMomentum(PhysxManager::vec3_to_nxVec3(momentum));
}

/**
 * Retrieves the linear momentum of an actor.  The momentum is equal to the
 * velocity times the mass.  The actor must be dynamic.
 */
LVector3f PhysxActor::
get_linear_momentum() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getLinearMomentum());
}

/**
 * Retrieves the angular momentum of an actor.  The angular momentum is equal
 * to the angular velocity times the global space inertia tensor.  The actor
 * must be dynamic.
 */
LVector3f PhysxActor::
get_angular_momentum() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getAngularMomentum());
}

/**
 * Sets the linear velocity below which an actor may go to sleep.  Actors
 * whose linear velocity is above this threshold will not be put to sleep.
 *
 * Setting the sleep angular/linear velocity only makes sense when the
 * BF_energy_sleep_test is not set.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
set_sleep_linear_velocity(float threshold) {

  nassertv(_error_type == ET_ok);
  _ptr->setSleepLinearVelocity(threshold);
}

/**
 * Sets the angular velocity below which an actor may go to sleep.  Actors
 * whose angular velocity is above this threshold will not be put to sleep.
 *
 * Setting the sleep angular/linear velocity only makes sense when the
 * BF_energy_sleep_test is not set.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
set_sleep_angular_velocity(float threshold) {

  nassertv(_error_type == ET_ok);
  _ptr->setSleepAngularVelocity(threshold);
}

/**
 * Sets the energy threshold below which an actor may go to sleep.  Actors
 * whose kinematic energy is above this threshold will not be put to sleep.
 *
 * Setting the sleep energy threshold only makes sense when the
 * BF_energy_sleep_test is set.  There are also other types of sleeping that
 * uses the linear and angular velocities directly instead of the energy.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
set_sleep_energy_threshold(float threshold) {

  nassertv(_error_type == ET_ok);
  _ptr->setSleepEnergyThreshold(threshold);
}

/**
 * Returns the linear velocity below which an actor may go to sleep.  Actors
 * whose linear velocity is above this threshold will not be put to sleep.
 * The actor must be dynamic.
 */
float PhysxActor::
get_sleep_linear_velocity() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getSleepLinearVelocity();
}

/**
 * Returns the angular velocity below which an actor may go to sleep.  Actors
 * whose angular velocity is above this threshold will not be put to sleep.
 * The actor must be dynamic.
 */
float PhysxActor::
get_sleep_angular_velocity() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getSleepAngularVelocity();
}

/**
 * Returns the energy below which an actor may go to sleep.  Actors whose
 * energy is above this threshold will not be put to sleep.  The actor must be
 * dynamic.
 */
float PhysxActor::
get_sleep_energy_threshold() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getSleepEnergyThreshold();
}

/**
 * Returns true if this body is sleeping.
 *
 * When an actor does not move for a period of time, it is no longer simulated
 * in order to save time.  This state is called sleeping.  However, because
 * the object automatically wakes up when it is either touched by an awake
 * object, or one of its properties is changed by the user, the entire sleep
 * mechanism should be transparent to the user.
 *
 * The actor must be dynamic.
 */
bool PhysxActor::
is_sleeping() const {

  nassertr(_error_type == ET_ok, false);
  return _ptr->isSleeping();
}

/**
 * Wakes up the actor if it is sleeping.
 *
 * The wakeCounterValue determines how long until the body is put to sleep, a
 * value of zero means that the body is sleeping.  wake_up(0) is equivalent to
 * PhysxActor::put_to_sleep().
 *
 * The actor must be dynamic.
 */
void PhysxActor::
wake_up(float wakeCounterValue) {

  nassertv(_error_type == ET_ok);
  _ptr->wakeUp(wakeCounterValue);
}

/**
 * Forces the actor to sleep.
 *
 * The actor will stay asleep until the next call to simulate, and will not
 * wake up until then even when otherwise it would (for example a force is
 * applied to it). It can however wake up during the next do_physics call.
 *
 * The actor must be dynamic.
 */
void PhysxActor::
put_to_sleep() {

  nassertv(_error_type == ET_ok);
  _ptr->putToSleep();
}

/**
 * Sets the mass of a dynamic actor.
 */
void PhysxActor::
set_mass(float mass) {

  nassertv(_error_type == ET_ok);
  _ptr->setMass(mass);
}

/**
 * Returns the mass of the actor.
 */
float PhysxActor::
get_mass() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getMass();
}

/**
 * Sets the matrix of the center of mass relative to the actor.
 */
void PhysxActor::
set_c_mass_offset_local_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassOffsetLocalPose(PhysxManager::mat4_to_nxMat34(mat));
}

/**
 * Sets the position of the center of mass relative to the actor.
 */
void PhysxActor::
set_c_mass_offset_local_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassOffsetLocalPosition(PhysxManager::point3_to_nxVec3(pos));
}

/**
 * Sets the orientation of the center of mass relative to the actor.
 */
void PhysxActor::
set_c_mass_offset_local_orientation(const LMatrix3f &mat) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassOffsetLocalOrientation(PhysxManager::mat3_to_nxMat33(mat));
}

/**
 * Sets the matrix of the center of mass relative to world space.
 */
void PhysxActor::
set_c_mass_offset_global_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassOffsetGlobalPose(PhysxManager::mat4_to_nxMat34(mat));
}

/**
 * Sets the position of the center of mass relative to world space.
 */
void PhysxActor::
set_c_mass_offset_global_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassOffsetGlobalPosition(PhysxManager::point3_to_nxVec3(pos));
}

/**
 * Sets the orientation of the center of mass relative to world space.
 */
void PhysxActor::
set_c_mass_offset_global_orientation(const LMatrix3f &mat) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassOffsetGlobalOrientation(PhysxManager::mat3_to_nxMat33(mat));
}

/**
 * Moves the actor by setting the transform of the center of mass.
 */
void PhysxActor::
set_c_mass_global_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassGlobalPose(PhysxManager::mat4_to_nxMat34(mat));
}

/**
 * Moves the actor by setting the position of the center of mass.
 */
void PhysxActor::
set_c_mass_global_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassGlobalPosition(PhysxManager::point3_to_nxVec3(pos));
}

/**
 * Moves the actor by setting the orientation of the center of mass.
 */
void PhysxActor::
set_c_mass_global_orientation(const LMatrix3f &mat) {

  nassertv(_error_type == ET_ok);
  _ptr->setCMassGlobalOrientation(PhysxManager::mat3_to_nxMat33(mat));
}

/**
 * Sets the inertia tensor, using a parameter specified in mass space
 * coordinates.
 */
void PhysxActor::
set_mass_space_inertia_tensor(const LVector3f &m) {

  nassertv(_error_type == ET_ok);
  _ptr->setMassSpaceInertiaTensor(PhysxManager::vec3_to_nxVec3(m));
}

/**
 * Returns the center of mass transform in world space.
 */
LMatrix4f PhysxActor::
get_c_mass_global_mat() const {

  nassertr(_error_type == ET_ok, LMatrix4f::zeros_mat());
  return PhysxManager::nxMat34_to_mat4(_ptr->getCMassGlobalPose());
}

/**
 * Returns the center of mass position in world space.
 */
LPoint3f PhysxActor::
get_c_mass_global_pos() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());
  return PhysxManager::nxVec3_to_point3(_ptr->getCMassGlobalPosition());
}

/**
 * Returns the center of mass orientation in world space.
 */
LMatrix3f PhysxActor::
get_c_mass_global_orientation() const {

  nassertr(_error_type == ET_ok, LMatrix3f::ident_mat());
  return PhysxManager::nxMat33_to_mat3(_ptr->getCMassGlobalOrientation());
}

/**
 * Returns the center of mass transform relative to the actor.
 */
LMatrix4f PhysxActor::
get_c_mass_local_mat() const {

  nassertr(_error_type == ET_ok, LMatrix4f::zeros_mat());
  return PhysxManager::nxMat34_to_mat4(_ptr->getCMassLocalPose());
}

/**
 * Returns the center of mass position relative to the actor.
 */
LPoint3f PhysxActor::
get_c_mass_local_pos() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());
  return PhysxManager::nxVec3_to_point3(_ptr->getCMassLocalPosition());
}

/**
 * Returns the center of mass orientation relative to the actor.
 */
LMatrix3f PhysxActor::
get_c_mass_local_orientation() const {

  nassertr(_error_type == ET_ok, LMatrix3f::ident_mat());
  return PhysxManager::nxMat33_to_mat3(_ptr->getCMassLocalOrientation());
}

/**
 * Returns the diagonal inertia tensor of the actor relative to the mass
 * coordinate frame.
 */
LVector3f PhysxActor::
get_mass_space_inertia_tensor() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getMassSpaceInertiaTensor());
}

/**
 * Returns the inertia tensor of the actor relative to the world coordinate
 * frame.
 */
LMatrix3f PhysxActor::
get_global_inertia_tensor() const {

  nassertr(_error_type == ET_ok, LMatrix3f::ident_mat());
  return PhysxManager::nxMat33_to_mat3(_ptr->getGlobalInertiaTensor());
}

/**
 * Returns the inverse of the inertia tensor of the actor relative to the
 * world coordinate frame.
 */
LMatrix3f PhysxActor::
get_global_inertia_tensor_inverse() const {

  nassertr(_error_type == ET_ok, LMatrix3f::ident_mat());
  return PhysxManager::nxMat33_to_mat3(_ptr->getGlobalInertiaTensorInverse());
}
